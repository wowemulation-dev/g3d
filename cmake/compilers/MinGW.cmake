# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright 2024, WoW Emulation and contributors
#
# This provides default settings for MinGW (Minimalist GNU for Windows).
set(PROJECT_COMPILER "${CMAKE_CXX_COMPILER_ID}")

# -----------------------------------------------------------------------------
# Compiler specific dependencies

# -----------------------------------------------------------------------------
# Compiler specific definitions
add_definitions(-DFREE_WINDOWS)

# -----------------------------------------------------------------------------
# Compiler specific flags

# -----------------------------------------------------------------------------
message(STATUS "${PROJECT_COMPILER}: compiler configuration finished.")
