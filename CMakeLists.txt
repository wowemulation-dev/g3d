# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright 2024, WoW Emulation and contributors

cmake_minimum_required(VERSION 3.22...3.28)

# ---- Project ----

project(
  G3D
  VERSION 9.0.0
  LANGUAGES CXX
)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/macros")
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/packages")

# ---- Add dependencies via CPM ------------------------------------------------
# see https://github.com/TheLartians/CPM.cmake for more info
set(CPM_USE_LOCAL_PACKAGES "ON")
include(CPM)

# Format.cmake will be used to add code formatting staged for clang
# and cmake.
CPMAddPackage("gh:TheLartians/Format.cmake@1.8.1")

# PackageProject.cmake will be used to make our target installable see
# https://github.com/TheLartians/PackageProject.cmake for more info
CPMAddPackage("gh:TheLartians/PackageProject.cmake@1.11.2")
CPMAddPackage("gh:nih-at/libzip@1.10.1")

find_package(ZLIB REQUIRED)

# ---- Add helpers for static analysis -----------------------------------------
include(Tools)

add_subdirectory(G3D.lib)
