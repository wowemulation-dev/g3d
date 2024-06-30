# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright 2024, WoW Emulation and contributors
#

# This function will check if the build should be executed in the applications source directory. If
# so, it will prevent the build and advise on proper build strategies.
function(AssureOutOfSourceBuilds)
  # make sure the user doesn't play dirty with symlinks
  get_filename_component(srcdir "${CMAKE_SOURCE_DIR}" REALPATH)
  get_filename_component(bindir "${CMAKE_BINARY_DIR}" REALPATH)

  # disallow in-source builds
  if("${srcdir}" STREQUAL "${bindir}")
    message("######################################################")
    message("# TrinityCore should not be configured & built in the source directory.")
    message("# You must run CMake in a build directory.")
    message("# For example:")
    message("# mkdir TrinityCore-Sandbox ; cd TrinityCore-sandbox")
    message(
      "# git clone https://gitlab.com/wowstack/trinity-core.git # or download & unpack the source tarball"
    )
    message("# mkdir TrinityCore-build")
    message("# this will create the following directory structure")
    message("#")
    message("# TrinityCore-Sandbox")
    message("#  +--TrinityCore")
    message("#  +--TrinityCore-build")
    message("#")
    message("# Then you can proceed to configure and build")
    message("# by using the following commands")
    message("#")
    message("# cd TrinityCore-build")
    message("# cmake ../TrinityCore # or ccmake, or cmake-gui ")
    message("# make")
    message("#")
    message("# NOTE: Given that you already tried to make an in-source build")
    message("#       CMake have already created several files & directories")
    message("#       in your source tree. run 'git status' to find them and")
    message("#       remove them by doing:")
    message("#")
    message("#       cd TrinityCore-Sandbox/TrinityCore")
    message("#       git clean -n -d")
    message("#       git clean -f -d")
    message("#       git checkout --")
    message("#")
    message("######################################################")
    message(FATAL_ERROR "Quitting configuration")
  endif()
endfunction()

assureoutofsourcebuilds()
