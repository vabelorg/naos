// Copyright (c) 2026 The Naos Authors
// SPDX-License-Identifier: MIT

#include <naos/naos.h>

#include <string>
#include <vector>

// A test that may stop leaves by longjmp, which runs no destructor: objects with one live outside
// it, made in setup and destroyed in teardown.
static std::vector<std::string> *words;

NAOS_SETUP(setup)
{
    words = new std::vector<std::string>();
}

NAOS_TEARDOWN(teardown)
{
    delete words;
    words = nullptr;
}

NAOS_TEST(push_back_adds)
{
    words->push_back("naos");

    NAOS_REQUIRE_UINT_EQ(1, words->size()); // may stop the test: no local object with a destructor
    NAOS_CHECK_STR_EQ("naos", (*words)[0].c_str());
}

// NAOS_CHECK* never stops the test, so a local std::string is fine here.
NAOS_TEST(only_checks)
{
    std::string word = "harness";

    NAOS_CHECK_UINT_EQ(7, word.size());
    NAOS_CHECK_STR_EQ("harness", word.c_str());
}

NAOS_SUITE(words,
    NAOS_SET_SETUP(setup),
    NAOS_SET_TEARDOWN(teardown),
    NAOS_ADD_TEST(push_back_adds),
    NAOS_ADD_TEST(only_checks)
)

NAOS_MAIN(
    NAOS_ADD_SUITE(words)
)
