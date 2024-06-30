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

# ---- Configure default build type to "Release" -------------------------------
set(CMAKE_BUILD_TYPE_INIT "Release")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE
        "Release"
        CACHE
            STRING
            "Choose the type of build, options are: Debug, Release, RelWithDebInfo and MinSizeRel"
            FORCE)
endif()

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

# ---- Print configuration summary initial run ---------------------------------
if(NOT EXECUTABLE_OUTPUT_PATH)
    set(PROJECT_FIRST_RUN "TRUE")
endif()

# ---- Redirect output files ---------------------------------------------------
set(EXECUTABLE_OUTPUT_PATH
    ${CMAKE_BINARY_DIR}/bin
    CACHE INTERNAL "" FORCE)
set(LIBRARY_OUTPUT_PATH
    ${CMAKE_BINARY_DIR}/lib
    CACHE INTERNAL "" FORCE)

# ---- Configure build architecture and platform -------------------------------
include(CheckArchitecture)
target_architecture(PROJECT_ARCHITECTURE)

include(CheckPlatform)
target_platform(PROJECT_PLATFORM)

# ---- Include the platform-specific file --------------------------------------
# To allow exceptions, this code looks for files in order of how specific they
# are.
#
# If there is, for example, a generic Linux.cmake and a version-specific
# Linux-6.8.0, it will pick this version and include it. It is then up to the
# file writer to include the generic version if necessary.
message(STATUS "")
foreach(
    _base ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_VERSION}-${CMAKE_SYSTEM_PROCESSOR}
          ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_VERSION} ${CMAKE_SYSTEM_NAME})
    set(_file ${CMAKE_CURRENT_SOURCE_DIR}/cmake/platforms/${_base}.cmake)
    if(EXISTS ${_file})
        include(${_file})
        break()
    endif()
endforeach()

# ---- Include the compiler-specific file --------------------------------------
# To allow exceptions, this code looks for files in order of how specific they
# are.
# If there is, for example, a generic GNU.cmake and a version-specific
# GNU-12.0.0.cmake, it will pick this version and include it. It is then up to
# the file writer to include the generic version if necessary.
message(STATUS "")
foreach(_base ${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}
              ${CMAKE_CXX_COMPILER_ID})
    set(_file ${CMAKE_CURRENT_SOURCE_DIR}/cmake/compilers/${_base}.cmake)
    if(EXISTS ${_file})
        include(${_file})
        break()
    endif()
endforeach()

include(ProjectInfo)

add_subdirectory(G3D.lib)
