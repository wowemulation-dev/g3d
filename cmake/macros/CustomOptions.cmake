# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright 2024, WoW Emulation and contributors
#
include(CMakeDependentOption)

# set_option(<_name> <_desc> <_default>): Set a given option <_name> with <_description> and an
# optional default value of <_default>.
macro(SET_OPTION _NAME _DESC)
  list(APPEND ALLOPTIONS ${_NAME})
  if(${ARGC} EQUAL 3)
    set(_DEFLT ${ARGV2})
  else()
    set(_DEFLT OFF)
  endif()
  option(${_NAME} ${_DESC} ${_DEFLT})
endmacro()

# dep_option(<_name> <_description> <_default> <_dependent_option> <_fail_default>): Set a given
# option <_name> with <_description> to a value of <_default>. If <_dependent_option> is OFF, the
# options value will be set to <_fail_default>.
macro(DEP_OPTION _NAME _DESC _DEFLT _DEPTEST _FAILDFLT)
  list(APPEND ALLOPTIONS ${_NAME})
  cmake_dependent_option(${_NAME} ${_DESC} ${_DEFLT} ${_DEPTEST} ${_FAILDFLT})
endmacro()

# option_string(<_name> <_description> <_value>): Set an option list <_name> with a given
# <_description> to <_value>.
macro(OPTION_STRING _NAME _DESC _VALUE)
  list(APPEND ALLOPTIONS ${_NAME})
  set(${_NAME}
      ${_VALUE}
      CACHE STRING "${_DESC}"
  )
  set(HAVE_${_NAME} ${_VALUE})
endmacro()
