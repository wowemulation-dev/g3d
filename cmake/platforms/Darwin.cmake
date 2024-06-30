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
# Configure uninstaller
configure_file(
  "${PROJECT_SOURCE_DIR}/cmake/macros/UninstallTargets.cmake"
  "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake" @ONLY
)

message(STATUS "Darwin: configuring uninstall target.")

# -----------------------------------------------------------------------------
# create uninstaller target (allows for using "make uninstall")
add_custom_target(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake")

message(STATUS "Darwin: created uninstall target.")

# -----------------------------------------------------------------------------
message(STATUS "Darwin: platform configuration finished.")
