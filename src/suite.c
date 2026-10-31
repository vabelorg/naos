/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * naos/suite.h: registering the suites and their tests, and finding every mistake in them before
 * the run (suite_internal.h).
 */

#include <naos/suite.h>

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <naos/exit.h>

#include "report.h"
#include "suite_internal.h"

/*
 * The suite functions given to NAOS_ADD_SUITE, in order, until naos_run() calls them. Allocated with
 * malloc as the builder is, and released by naos_suite_release().
 */
typedef struct naos_added
{
    naos_suite_func_t *suites;
    size_t count;
    size_t capacity;
} naos_added_t;

static void builder_error(naos_builder_t *builder, const char *format, ...) NAOS_PRINTF_FORMAT(2, 3);

static naos_added_t g_added = { NULL, 0, 0 };

/*
 * Set once when adding or collecting the suites runs out of memory; every later registration does
 * nothing, and naos_run() reports it in one line.
 */
static int g_out_of_memory = 0;

/* Whether c is an ASCII letter, a digit or '_', whatever the locale. */
static int is_name_char(char c)
{
    if (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')))
    {
        return 1;
    }

    return (((c >= '0') && (c <= '9')) || (c == '_')) ? 1 : 0;
}

/*
 * Accepts a suite or test name made of letters, digits and '_' only, as the macros make them: the
 * name <suite>.<test> then splits at its one '.', and none of its characters means anything to
 * --naos-filter ('*', '?', ':', '-') or to the command line of CTest.
 */
static int valid_name(const char *name)
{
    const char *c;

    if ((name == NULL) || (name[0] == '\0'))
    {
        return 0;
    }

    for (c = name; *c != '\0'; c++)
    {
        if (is_name_char(*c) == 0)
        {
            return 0;
        }
    }

    return 1;
}

/* A name as the messages show it: (null) and (empty) are never valid names, so they cannot be mistaken for one. */
static const char *display_name(const char *name)
{
    if (name == NULL)
    {
        return "(null)";
    }

    return (name[0] != '\0') ? name : "(empty)";
}

/*
 * Prints a mistake in the suites after "naos: ", formatted as printf does, and counts it; naos_run()
 * stops before running once all are reported.
 */
static void builder_error(naos_builder_t *builder, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    naos_report_error_v(format, args);
    va_end(args);

    builder->error_count++;
}

/*
 * Makes room for one more element in items, an array of count elements of size bytes with room for
 * *capacity, by doubling the capacity from 8. Returns the array, moved or not, or NULL when the
 * memory runs out; then items and *capacity are left as they were.
 */
static void *grow_array(void *items, size_t count, size_t *capacity, size_t size)
{
    void *grown;
    size_t wanted;

    if (count < *capacity)
    {
        return items;
    }

    wanted = (*capacity == 0) ? 8 : (*capacity * 2);
    grown = NULL;
    if ((wanted > *capacity) && (wanted <= (SIZE_MAX / size)))
    {
        grown = realloc(items, wanted * size);
    }

    if (grown == NULL)
    {
        return NULL;
    }

    *capacity = wanted;

    return grown;
}

/*
 * Gets the suite being registered, or NULL when nothing is to be recorded: after a lack of memory,
 * or when the suite function has not begun its suite (reported as a mistake).
 */
static naos_suite_t *builder_suite(naos_builder_t *builder, const char *what)
{
    if ((builder == NULL) || (g_out_of_memory != 0))
    {
        return NULL;
    }

    if (builder->current == NULL)
    {
        builder_error(builder, "suite #%zu calls %s before naos_begin_suite", builder->count - 1, what);
        return NULL;
    }

    return builder->current;
}

/* Sets the setup or the teardown of suite once; what names it in the messages. */
static void set_fixture(naos_builder_t *builder, const naos_suite_t *suite, naos_func_t *slot, naos_func_t func, const char *what)
{
    if (func == NULL)
    {
        builder_error(builder, "%s of %s is NULL", what, display_name(suite->name));
    }
    else if (*slot != NULL)
    {
        builder_error(builder, "%s of %s is set twice", what, display_name(suite->name));
    }
    else
    {
        *slot = func;
    }
}

/* naos_suite_collect() of the suite functions suites[0, count). */
static int build_suites(naos_builder_t *builder, const naos_suite_func_t *suites, size_t count)
{
    size_t i;
    size_t j;

    builder->suites = NULL;
    builder->count = 0;
    builder->current = NULL;
    builder->test_calls = 0;
    builder->error_count = 0;

    /* g_out_of_memory may be set already, by NAOS_ADD_SUITE. */
    if ((count != 0) && (g_out_of_memory == 0))
    {
        builder->suites = (naos_suite_t *)calloc(count, sizeof(naos_suite_t));
        g_out_of_memory = (builder->suites == NULL) ? 1 : 0;
    }

    for (i = 0; (i < count) && (g_out_of_memory == 0); i++)
    {
        builder->count = i + 1;
        builder->current = NULL;

        if (suites[i] == NULL)
        {
            builder_error(builder, "suite #%zu is NULL", i);
            continue;
        }

        suites[i](builder);

        if ((g_out_of_memory == 0) && (builder->current == NULL))
        {
            builder_error(builder, "suite #%zu does not call naos_begin_suite", i);
        }
    }

    builder->current = NULL;

    if (g_out_of_memory != 0)
    {
        naos_report_error("out of memory while preparing the suites");

        return NAOS_EXIT_FAIL;
    }

    /* Only now are all the names known; invalid ones have been reported already. */
    for (i = 0; i < count; i++)
    {
        if (valid_name(builder->suites[i].name) == 0)
        {
            continue;
        }

        for (j = 0; j < i; j++)
        {
            if ((valid_name(builder->suites[j].name) != 0) && (strcmp(builder->suites[i].name, builder->suites[j].name) == 0))
            {
                builder_error(builder, "duplicate suite name: %s", builder->suites[i].name);
                break;
            }
        }
    }

    return (builder->error_count == 0) ? NAOS_EXIT_PASS : NAOS_EXIT_USAGE;
}

void naos_begin_suite(naos_builder_t *builder, const char *name)
{
    naos_suite_t *suite;

    if ((builder == NULL) || (g_out_of_memory != 0))
    {
        return;
    }

    if (builder->current != NULL)
    {
        builder_error(builder, "suite #%zu begins a suite twice", builder->count - 1);
        return;
    }

    suite = &builder->suites[builder->count - 1];
    suite->name = name;
    builder->current = suite;
    builder->test_calls = 0;

    if (valid_name(name) == 0)
    {
        builder_error(builder, "suite #%zu has an invalid name (only letters, digits and '_'): %s", builder->count - 1, display_name(name));
    }
}

void naos_add_test(naos_builder_t *builder, const char *name, naos_func_t func)
{
    naos_suite_t *suite;
    naos_test_t *grown;
    size_t index;
    size_t i;

    suite = builder_suite(builder, "naos_add_test");
    if (suite == NULL)
    {
        return;
    }

    /* Counts every call, so that the messages point at the entry of the suite, accepted or not. */
    index = builder->test_calls++;

    if (valid_name(name) == 0)
    {
        builder_error(builder, "test #%zu of %s has an invalid name (only letters, digits and '_'): %s", index, display_name(suite->name), display_name(name));
        return;
    }

    if (func == NULL)
    {
        builder_error(builder, "test %s.%s has no function", display_name(suite->name), name);
        return;
    }

    for (i = 0; i < suite->count; i++)
    {
        if (strcmp(name, suite->tests[i].name) == 0)
        {
            builder_error(builder, "duplicate test name: %s.%s", display_name(suite->name), name);
            return;
        }
    }

    grown = (naos_test_t *)grow_array(suite->tests, suite->count, &suite->capacity, sizeof(naos_test_t));
    if (grown == NULL)
    {
        g_out_of_memory = 1;
        return;
    }

    suite->tests = grown;
    suite->tests[suite->count].name = name;
    suite->tests[suite->count].func = func;
    suite->count++;
}

void naos_set_setup(naos_builder_t *builder, naos_func_t func)
{
    naos_suite_t *suite;

    suite = builder_suite(builder, "naos_set_setup");
    if (suite != NULL)
    {
        set_fixture(builder, suite, &suite->setup, func, "setup");
    }
}

void naos_set_teardown(naos_builder_t *builder, naos_func_t func)
{
    naos_suite_t *suite;

    suite = builder_suite(builder, "naos_set_teardown");
    if (suite != NULL)
    {
        set_fixture(builder, suite, &suite->teardown, func, "teardown");
    }
}

void naos_suite_keep(naos_suite_func_t suite)
{
    naos_suite_func_t *grown;

    if (g_out_of_memory != 0)
    {
        return;
    }

    grown = (naos_suite_func_t *)grow_array(g_added.suites, g_added.count, &g_added.capacity, sizeof(naos_suite_func_t));
    if (grown == NULL)
    {
        g_out_of_memory = 1;
        return;
    }

    g_added.suites = grown;

    /* NULL is kept too, so that naos_suite_collect() reports it with the other mistakes. */
    g_added.suites[g_added.count] = suite;
    g_added.count++;
}

int naos_suite_collect(naos_builder_t *builder)
{
    return build_suites(builder, g_added.suites, g_added.count);
}

void naos_suite_release(naos_builder_t *builder)
{
    size_t i;

    if (builder->suites != NULL)
    {
        for (i = 0; i < builder->count; i++)
        {
            free(builder->suites[i].tests);
        }

        free(builder->suites);
    }

    builder->suites = NULL;
    builder->count = 0;

    free(g_added.suites);
    g_added.suites = NULL;
    g_added.count = 0;
    g_added.capacity = 0;
}
