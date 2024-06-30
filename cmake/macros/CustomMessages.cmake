# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright 2024, WoW Emulation and contributors
#
include(CMakeDependentOption)

# message_warn(<text>): Displays a given <text> variable or string with a warning note.
macro(MESSAGE_WARN _TEXT)
  message(STATUS "*** WARNING: ${_TEXT}")
endmacro()

# message_error(<text>): Displays a given <text> variable or string with an error note and aborts
# the build process.
macro(MESSAGE_ERROR _TEXT)
  message(FATAL_ERROR "*** ERROR: ${_TEXT}")
endmacro()

# message_bool_option(<option_name> <option_value>): Displays <option_name> as ON/OFF switch based
# on <option_value>.
macro(MESSAGE_BOOL_OPTION _NAME _VALUE)
  set(_PAD " ")
  if(${ARGC} EQUAL 3)
    set(_PAD ${ARGV2})
  endif()
  if(${_VALUE})
    message(STATUS "${_NAME}:${_PAD}ON")
  else()
    message(STATUS "${_NAME}:${_PAD}OFF")
  endif()
endmacro()

macro(MESSAGE_TESTED_OPTION _NAME)
  set(_REQVALUE ${${_NAME}})
  set(_PAD " ")
  if(${ARGC} EQUAL 2)
    set(_PAD ${ARGV1})
  endif()
  if(NOT HAVE_${_NAME})
    set(HAVE_${_NAME} OFF)
  elseif("${HAVE_${_NAME}}" MATCHES "1|TRUE|YES|Y")
    set(HAVE_${_NAME} ON)
  endif()
  message(STATUS "  ${_NAME}${_PAD}(Wanted: ${_REQVALUE}): ${HAVE_${_NAME}}")
endmacro()
