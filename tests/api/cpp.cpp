/*
 * Copyright (c) 2026 The Naos Authors
 * SPDX-License-Identifier: MIT
 *
 * Passing tests written in C++: the macros of naos.h expand to valid C++11, without warnings, and
 * link against the C library through extern "C". NAOS_REQUIRE_* leave through longjmp, so objects
 * with destructors live at namespace scope rather than in the frames they could jump over.
 */

#include <naos/naos.h>

#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace
{
std::vector<int> values;
int setup_calls = 0;
const std::string text("abc");
} // namespace

NAOS_SETUP(setup)
{
    values.assign(3, 7);
    setup_calls++;
}

NAOS_TEARDOWN(teardown)
{
    values.clear();
}

static void require_in_helper(int value)
{
    NAOS_REQUIRE_INT_GE(value, 0);
}

/* Ends with NAOS_FAIL rather than return, without -Wreturn-type or C4715. */
static int find_index(int value)
{
    if (value == 7)
    {
        return 0;
    }

    NAOS_FAIL("not found");
}

NAOS_TEST(assertions)
{
    const char *none = nullptr;
    int value = 0;

    NAOS_CHECK_TRUE(text.size() == 3);
    NAOS_REQUIRE_TRUE(!values.empty());
    NAOS_CHECK_FALSE(values.empty());
    NAOS_REQUIRE_FALSE(text.empty());
    NAOS_CHECK_INT_EQ(-1, -1);
    NAOS_CHECK_INT_NE(1, 2);
    NAOS_CHECK_INT_LT(INTMAX_MIN, 0);
    NAOS_CHECK_INT_LE(0, 0);
    NAOS_CHECK_INT_GT(1, 0);
    NAOS_CHECK_INT_GE(1, 1);
    NAOS_REQUIRE_UINT_EQ(values.size(), 3u);
    NAOS_CHECK_UINT_NE(0u, UINTMAX_MAX);
    NAOS_CHECK_UINT_LT(values.size(), 4u);
    NAOS_CHECK_UINT_LE(values.size(), 3u);
    NAOS_CHECK_UINT_GT(values.size(), 2u);
    NAOS_CHECK_UINT_GE(values.size(), 3u);
    NAOS_CHECK_DOUBLE_NEAR(0.3, 0.1 + 0.2, 1e-12);
    NAOS_CHECK_DOUBLE_NEAR(HUGE_VAL, HUGE_VAL, 0.0);
    NAOS_CHECK_STR_EQ("abc", text.c_str());
    NAOS_CHECK_STR_NE("abd", text.c_str());
    NAOS_CHECK_MEM_EQ("abc", 3, text.data(), text.size());
    NAOS_CHECK_PTR_EQ(&value, &value);
    NAOS_CHECK_PTR_EQ(nullptr, none);
    NAOS_CHECK_NULL(none);
    NAOS_CHECK_NOT_NULL(&value);
    NAOS_REQUIRE_NOT_NULL(std::strchr(text.c_str(), 'b'));
    NAOS_CHECK_STR_EQ(NAOS_VERSION_STRING, naos_version_string());
    NAOS_CHECK_UINT_EQ(NAOS_VERSION_NUMBER, naos_version_number());
}

NAOS_TEST(note_and_helpers)
{
    for (std::size_t i = 0; i < values.size(); i++)
    {
        NAOS_TRACE_PUSH("i=%zu", i);
        NAOS_NOTE("i=%zu", i);
        NAOS_CHECK_INT_EQ(7, values[i]);
        NAOS_TRACE_POP();
    }

    NAOS_TRACE_PUSH("without arguments");
    NAOS_TRACE_POP();
    NAOS_NOTE("without arguments");
    require_in_helper(1);
    require_in_helper(2);
    NAOS_CHECK_INT_EQ(0, find_index(values[0]));
}

NAOS_TEST(setup_runs_before_each_test)
{
    NAOS_CHECK_INT_EQ(3, values.size());
    NAOS_CHECK_INT_GE(setup_calls, 1);
}

NAOS_SUITE(cpp,
    NAOS_SET_SETUP(setup),
    NAOS_SET_TEARDOWN(teardown),
    NAOS_ADD_TEST(assertions),
    NAOS_ADD_TEST(note_and_helpers),
    NAOS_ADD_TEST(setup_runs_before_each_test)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(cpp)
)
