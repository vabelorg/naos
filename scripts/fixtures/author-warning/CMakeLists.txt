# Copyright (c) 2026 The Naos Authors
# SPDX-License-Identifier: MIT
#
# A project with a warning for the authors of a project and nothing else. Its configure must fail
# with the option that scripts/TestPackage.cmake picks, or that option has no effect.

cmake_minimum_required(VERSION 3.21)
project(naos_author_warning LANGUAGES NONE)

message(AUTHOR_WARNING "this warning must be an error")
