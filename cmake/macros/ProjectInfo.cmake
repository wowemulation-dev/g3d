# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright 2024, WoW Emulation and contributors
#
include(CustomMessages)

if(PROJECT_FIRST_RUN)
  message(STATUS "")
  # --------------------------------------------------------------------------
  # Build summary
  message(STATUS "Build summary for ${PROJECT_NAME}")
  message(STATUS "")

  # --------------------------------------------------------------------------
  # Version information
  message(STATUS "Version:        ${PROJECT_VERSION}")
  message(STATUS "")

  # --------------------------------------------------------------------------
  # System information
  message(STATUS "Target platform information")
  message(STATUS "Architecture:   ${PROJECT_ARCHITECTURE}")
  message(STATUS "Platform:       ${PROJECT_PLATFORM}")
  message(STATUS "")

  # --------------------------------------------------------------------------
  # Compilation and linker settings
  message(STATUS "Build information")
  message(STATUS "Compiler:       ${PROJECT_COMPILER}")
  message(STATUS "Build type:     ${CMAKE_BUILD_TYPE}")
  message(STATUS "")
endif()
