# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright 2024, WoW Emulation and contributors
#
# This file includes OSX specific options and quirks, related to system checks.

# -----------------------------------------------------------------------------
# Platform specific dependencies

# -----------------------------------------------------------------------------
# Platform specific definitions

# Set the rpath so that libraries are found
set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${INSTALL_LIBDIR})
set(CMAKE_INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/${INSTALL_LIBDIR})

# Run out of build tree
set(CMAKE_BUILD_WITH_INSTALL_RPATH OFF)

# -----------------------------------------------------------------------------
message(STATUS "Darwin: platform configuration finished.")
