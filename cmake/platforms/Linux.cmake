# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright 2024, WoW Emulation and contributors
#
# This file includes Linux specific options and quirks, related to system checks.

# -----------------------------------------------------------------------------
# Platform specific dependencies

# -----------------------------------------------------------------------------
# Platform specific definitions
add_definitions(-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE)

# Set the rpath so that libraries are found
set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${INSTALL_LIBDIR})
set(CMAKE_INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/${INSTALL_LIBDIR})

# Run out of build tree
set(CMAKE_BUILD_WITH_INSTALL_RPATH OFF)

# -----------------------------------------------------------------------------
message(STATUS "Linux: platform configuration finished.")
