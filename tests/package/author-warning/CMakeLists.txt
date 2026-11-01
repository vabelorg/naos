# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# A project with a warning for the authors of a project and nothing else. Its configure must fail
# with the option that tests/package/werror-option.cmake picks, or that option has no effect.

cmake_minimum_required(VERSION 3.21)
project(naos_author_warning LANGUAGES NONE)

message(AUTHOR_WARNING "this warning must be an error")
