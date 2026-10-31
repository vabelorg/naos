/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * The command line of naos_init() (options.h): parsing the --naos-* options, the usage line and
 * the help, all made from one table of the options.
 */

#include "options.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <naos/export.h>

#include "order.h"
#include "report.h"

/* What every Naos option starts with; the other arguments are left for the program. */
#define NAOS_OPTION_PREFIX "--naos-"

/* The one option without the prefix; it stays in argv so that the program can print its help too. */
#define NAOS_HELP_OPTION "--help"

/* The values of --naos-random-seed: the seeds of order.h, or 0 to choose one from the time. */
#define NAOS_RANDOM_SEED_RANGE "0 to " NAOS_ORDER_SEED_MAX_STRING

/* Width of the usage lines, and of the option column of the help. */
#define NAOS_USAGE_WIDTH 100
#define NAOS_HELP_COLUMN 28

/*
 * An entry of the option table, the one place an option is described: parsing, the usage line and
 * the help all read it. A flag sets *flag to 1; an option that takes a value, shown as value_name,
 * has it checked and stored by take_value, and what names it in the messages, such as "a pattern".
 * help is the text of the help, its lines separated by '\n'.
 */
typedef struct naos_option_entry
{
    const char *name;
    const char *value_name;
    const char *what;
    int *flag;
    void (*take_value)(const char *name, const char *value);
    const char *help;
} naos_option_entry_t;

static void take_filter(const char *name, const char *value);
static void take_random_seed(const char *name, const char *value);
static void take_repeat(const char *name, const char *value);
static void report_usage_error(const char *format, ...) NAOS_PRINTF_FORMAT(1, 2);

static naos_options_t g_options =
{
    NULL, /* program */
    NULL, /* filter */
    0,    /* list_only */
    0,    /* shuffle */
    0,    /* random_seed */
    1,    /* repeat */
    0,    /* fail_fast */
    0,    /* help_requested */
    0     /* usage_error */
};

/* In the order of the usage and the help. --help comes last; it is parsed apart, see naos_options_parse(). */
static const naos_option_entry_t g_option_table[] =
{
    {
        "--naos-filter", "<patterns>", "a pattern", NULL, take_filter,
        "run only the tests whose name <suite>.<test> matches\n"
        "'*' matches any string, '?' any one character;\n"
        "patterns are separated by ':', and those after '-' exclude:\n"
        "parser.*:writer.*-*.slow_*"
    },
    {
        "--naos-list", NULL, NULL, &g_options.list_only, NULL,
        "print the test names, one per line, without running them;\n"
        "the order of registration, whatever the options below"
    },
    {
        "--naos-shuffle", NULL, NULL, &g_options.shuffle, NULL,
        "run the suites, and the tests of each suite, in a random order"
    },
    {
        "--naos-random-seed", "<n>", "a number", NULL, take_random_seed,
        "seed of that order, 1 to " NAOS_ORDER_SEED_MAX_STRING "; 0 or none chooses one from the time"
    },
    {
        "--naos-repeat", "<n>", "a number", NULL, take_repeat,
        "run the tests n times; with --naos-shuffle, the seed goes up by 1\n"
        "each time"
    },
    {
        "--naos-fail-fast", NULL, NULL, &g_options.fail_fast, NULL,
        "stop at the first failing test, leaving the other tests and\n"
        "iterations not run"
    },
    {
        NAOS_HELP_OPTION, NULL, NULL, &g_options.help_requested, NULL,
        "print this help without running any test"
    }
};

#define NAOS_OPTION_COUNT (sizeof(g_option_table) / sizeof(g_option_table[0]))

/* Writes the name of option and, for one that takes a value, " <value>"; returns the length written. */
static size_t print_option_form(FILE *out, const naos_option_entry_t *option)
{
    if (option->value_name == NULL)
    {
        fputs(option->name, out);
        return strlen(option->name);
    }

    fprintf(out, "%s %s", option->name, option->value_name);

    return strlen(option->name) + 1 + strlen(option->value_name);
}

/* Writes the help text of option, each line after the first indented to NAOS_HELP_COLUMN. */
static void print_option_help(const char *help)
{
    const char *cursor;

    for (cursor = help; *cursor != '\0'; cursor++)
    {
        fputc(*cursor, stdout);
        if (*cursor == '\n')
        {
            printf("%*s", NAOS_HELP_COLUMN, "");
        }
    }

    fputc('\n', stdout);
}

static void print_help(const char *program)
{
    size_t length;
    size_t i;

    naos_options_print_usage(stdout, program);
    printf("\n");
    printf("options (anywhere on the command line):\n");
    for (i = 0; i < NAOS_OPTION_COUNT; i++)
    {
        fputs("  ", stdout);
        length = 2 + print_option_form(stdout, &g_option_table[i]);
        if ((length + 2) > NAOS_HELP_COLUMN)
        {
            /* Too long for the column: the help starts on the next line. */
            printf("\n%*s", NAOS_HELP_COLUMN, "");
        }
        else
        {
            printf("%*s", (int)(NAOS_HELP_COLUMN - length), "");
        }
        print_option_help(g_option_table[i].help);
    }
    printf("an option that takes a value also takes it after '=': --naos-filter=<patterns>.\n");
    printf("the other args are left in argv for the program.\n");
    fflush(stdout);
}

/* Prints a mistake in the arguments after "naos: ", formatted as printf does, and records it. */
static void report_usage_error(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    naos_report_error_v(format, args);
    va_end(args);

    g_options.usage_error = 1;
}

/*
 * Returns how arg starts with the option name: 0 when it does not, 1 when it is the name alone, and
 * 2 when "=" follows the name. A longer name, such as "--naos-filterx", does not match.
 */
static int option_form(const char *name, const char *arg)
{
    size_t length = strlen(name);

    if (strncmp(arg, name, length) != 0)
    {
        return 0;
    }

    if (arg[length] == '\0')
    {
        return 1;
    }

    return (arg[length] == '=') ? 2 : 0;
}

/*
 * Reads the value of the option name as a decimal number from min to max, digits only. Returns 1
 * with *number set, or 0 after reporting a usage error; what names the range in the message.
 */
static int parse_number(const char *name, const char *value, unsigned long min, unsigned long max, const char *what, unsigned long *number)
{
    const char *digit;

    for (digit = value; isdigit((unsigned char)*digit) != 0; digit++)
    {
        /* Skip the digits; the loop header does all the work. */
    }

    if ((digit != value) && (*digit == '\0'))
    {
        errno = 0;
        *number = strtoul(value, NULL, 10);
        if ((errno == 0) && (*number >= min) && (*number <= max))
        {
            return 1;
        }
    }

    report_usage_error("%s takes %s: %s", name, what, value);

    return 0;
}

static void take_filter(const char *name, const char *value)
{
    (void)name;
    g_options.filter = value;
}

static void take_random_seed(const char *name, const char *value)
{
    unsigned long number;

    if (parse_number(name, value, 0, NAOS_ORDER_SEED_MAX, NAOS_RANDOM_SEED_RANGE, &number) != 0)
    {
        g_options.random_seed = number;
    }
}

static void take_repeat(const char *name, const char *value)
{
    unsigned long number;

    if (parse_number(name, value, 1, LONG_MAX, "1 or more", &number) != 0)
    {
        g_options.repeat = (long)number;
    }
}

/* Finds the option arg names, setting *form as option_form() does, or returns NULL. */
static const naos_option_entry_t *find_option(const char *arg, int *form)
{
    size_t i;

    for (i = 0; i < NAOS_OPTION_COUNT; i++)
    {
        *form = option_form(g_option_table[i].name, arg);
        if (*form != 0)
        {
            return &g_option_table[i];
        }
    }

    return NULL;
}

/*
 * Takes the Naos option argv[*read]: a flag alone, or an option with its value given as
 * "<name> <value>" (moving *read past the value) or "<name>=<value>" (split at the first '=').
 * An unknown option, a value given to a flag (as in getopt_long) and a missing or empty value are
 * usage errors.
 */
static void take_option(int argc, char **argv, int *read)
{
    const naos_option_entry_t *option;
    const char *arg;
    const char *value;
    int form;

    arg = argv[*read];
    option = find_option(arg, &form);
    if (option == NULL)
    {
        report_usage_error("unknown option: %s", arg);
        return;
    }

    if (option->flag != NULL)
    {
        if (form == 2)
        {
            report_usage_error("%s does not take a value: %s", option->name, arg);
            return;
        }

        *option->flag = 1;
        return;
    }

    if (form == 2)
    {
        value = arg + strlen(option->name) + 1;
    }
    else if (*read + 1 < argc)
    {
        (*read)++;
        value = argv[*read];
    }
    else
    {
        report_usage_error("%s needs %s", option->name, option->what);
        return;
    }

    if (value[0] == '\0')
    {
        report_usage_error("%s needs %s, not an empty one", option->name, option->what);
        return;
    }

    option->take_value(option->name, value);
}

void naos_options_print_usage(FILE *out, const char *program)
{
    static const char lead[] = "usage: ";
    static const char rest[] = "[args...]";
    size_t column;
    size_t width;
    size_t i;

    if (program == NULL)
    {
        program = "naos";
    }

    fprintf(out, "%s%s", lead, program);
    column = (sizeof(lead) - 1) + strlen(program);

    /* Each "[<option>]" goes on the line it fits on, or else on a new one under the program name. */
    for (i = 0; i <= NAOS_OPTION_COUNT; i++)
    {
        width = (i < NAOS_OPTION_COUNT) ? (strlen(g_option_table[i].name) + 2) : (sizeof(rest) - 1);

        if ((i < NAOS_OPTION_COUNT) && (g_option_table[i].value_name != NULL))
        {
            width += 1 + strlen(g_option_table[i].value_name);
        }

        if ((column + 1 + width) > NAOS_USAGE_WIDTH)
        {
            fprintf(out, "\n%*s", (int)(sizeof(lead) - 1), "");
            column = sizeof(lead) - 1;
        }
        else
        {
            fputc(' ', out);
            column++;
        }

        if (i < NAOS_OPTION_COUNT)
        {
            fputc('[', out);
            print_option_form(out, &g_option_table[i]);
            fputc(']', out);
        }
        else
        {
            fputs(rest, out);
        }

        column += width;
    }

    fputc('\n', out);
}

void naos_options_parse(int *argc, char **argv)
{
    const char *arg;
    int read;
    int kept;

    /* Called once, so the options start from their static values. */
    if ((argc == NULL) || (*argc < 1) || (argv == NULL) || (argv[0] == NULL))
    {
        naos_options_print_usage(stderr, NULL);
        g_options.usage_error = 1;
        return;
    }

    g_options.program = argv[0];

    /* Takes the Naos options out wherever they are and keeps the rest in order. */
    kept = 1;
    for (read = 1; read < *argc; read++)
    {
        arg = argv[read];
        if (strncmp(arg, NAOS_OPTION_PREFIX, sizeof(NAOS_OPTION_PREFIX) - 1) == 0)
        {
            take_option(*argc, argv, &read);
            continue;
        }

        /* --help stays in argv so that the program can print its own help as well. */
        if (strcmp(arg, NAOS_HELP_OPTION) == 0)
        {
            g_options.help_requested = 1;
        }

        argv[kept++] = argv[read];
    }

    if (kept < *argc)
    {
        argv[kept] = NULL;
    }

    *argc = kept;

    if (g_options.usage_error != 0)
    {
        naos_options_print_usage(stderr, g_options.program);
    }
    else if (g_options.help_requested != 0)
    {
        print_help(g_options.program);
    }
}

const naos_options_t *naos_options_get(void)
{
    return &g_options;
}
