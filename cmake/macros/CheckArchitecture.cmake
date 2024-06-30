# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright 2024, WoW Emulation and contributors
#
# This file provides information on the build architecture for the current build.

# Based on the Qt 5 processor detection code, so should be very accurate
# https://qt.gitorious.org/qt/qtbase/blobs/master/src/corelib/global/qprocessordetection.h Currently
# handles arm (v5, v6, v7), x86 (32/64), ia64, and ppc (32/64)

# Regarding POWER/PowerPC, just as is noted in the Qt source, "There are many more known variants /
# revisions that we do not handle / detect."

set(archdetect_c_code
    "
    #if defined(__x86_64__) || defined(_M_X64)
    #error cmake_ARCH x86_64
#elif defined(__i386) || defined(_M_IX86)
    #error cmake_ARCH i686
#elif defined(__aarch64__) || defined(__arm64__) || defined(_M_ARM64) || defined(_M_ARM64EC)
    #error cmake_ARCH aarch64
#elif defined(__arm__) || defined(__arm) || defined(_M_ARM) || defined(__TARGET_ARCH_ARM)
    #if defined(__ARM64_ARCH_8__) || defined(__ARMv8__) || defined(__ARMv8_A__)
        #error cmake_ARCH armv8
    #elif defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__)
        #error cmake_ARCH armv7
    #elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6T2__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6ZK__) || defined(__ARM_ARCH_6M__)
        #error cmake_ARCH armv6
    #elif defined(__ARM_ARCH_5T__) || defined(__ARM_ARCH_5TE__) || defined(__ARM_ARCH_5TEJ__)
        #error cmake_ARCH armv5
    #elif defined(__ARM_ARCH_4T__) || defined(__TARGET_ARCH_5E__)
        #error cmake_ARCH armv4
    #elif defined(__ARM_ARCH_3__) || defined(__TARGET_ARCH_3M__)
        #error cmake_ARCH armv3
    #elif defined(__ARM_ARCH_2__)
        #error cmake_ARCH armv2
    #endif
#elif defined(__powerpc__) || defined(_ppc__) || defined(__PPC__)
    #if defined(__64BIT__) || defined(__powerpc64__) || defined(__ppc64__)
        #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            #error cmake_ARCH powerpc64le
        #else
            #error cmake_ARCH powerpc64
        #endif
    #else
        #error cmake_ARCH powerpc
    #endif
#elif defined(__alpha__) || defined(__alpha)
    #error cmake_ARCH alpha
#elif defined(__BFIN__)
    #error cmake_ARCH blackfin
#elif defined(__ia64) || defined(_M_IA64)
    #error cmake_ARCH ia64
#elif defined(__mips__) || defined(__mips)
    #error cmake_ARCH mips
#elif defined(__m68k__)
    #error cmake_ARCH m68k
#elif defined(__sh__)
    #error cmake_ARCH sh
#elif defined(__sparc__) || defined(__sparc)
    #if defined(__sparcv9) || defined(__sparc_v9__)
        #error cmake_ARCH sparc9
    #elif defined(__sparcv8) || defined(__sparc_v8__)
        #error cmake_ARCH sparc8
    #endif
#elif defined(__370__)
    #error cmake_ARCH s370
#elif defined(__s390__)
    #error cmake_ARCH s390
#elif defined(__s390x) || defined(__zarch__)
    #error cmake_ARCH s390x
#elif defined(__hppa__)
    #error cmake_ARCH parisc
#elif defined(__THW_RS6000)
    #error cmake_ARCH rs6000
#elif defined(__riscv)
    #if __riscv_xlen == 64
        #error cmake_ARCH riscv64
    #elif __riscv_xlen == 32
        #error cmake_ARCH riscv32
    #endif
#elif defined(__loongarch_lp64)
    #error cmake_ARCH loongarch64
#elif defined(__EMSCRIPTEN__)
    #error cmake_ARCH wasm32
#else
    #error cmake_ARCH unknown
#endif
"
)

# target_architecture(<output_var>): Determines the current CPU architecture and returns a unique
# architecture identifier.
#
# This function utilizes `try_run` and some preprocessor trickery to determine which architecture we
# are building on.
#
# Read more about architecture specific preprocessor definitions at
# http://sourceforge.net/p/predef/wiki/Home/
function(target_architecture output_var)
  if(APPLE AND CMAKE_OSX_ARCHITECTURES)
    # On OS X we use CMAKE_OSX_ARCHITECTURES *if* it was set First let's normalize the order of the
    # values

    # Note that it's not possible to compile PowerPC applications if you are using the OS X SDK
    # version 10.6 or later - you'll need 10.4/10.5 for that, so we disable it by default See this
    # page for more information:
    # http://stackoverflow.com/questions/5333490/how-can-we-restore-ppc-ppc64-as-well-as-full-10-4-10-5-sdk-support-to-xcode-4

    # Architecture defaults to i386 or ppc on OS X 10.5 and earlier, depending on the CPU type
    # detected at runtime. On OS X 10.6+ the default is x86_64 if the CPU supports it, i386
    # otherwise.

    foreach(osx_arch ${CMAKE_OSX_ARCHITECTURES})
      if("${osx_arch}" STREQUAL "ppc" AND ppc_support)
        set(osx_arch_ppc TRUE)
      elseif("${osx_arch}" STREQUAL "i386")
        set(osx_arch_i386 TRUE)
      elseif("${osx_arch}" STREQUAL "x86_64")
        set(osx_arch_x86_64 TRUE)
        elseif("${osx_arch}" STREQUAL "arm64")
        set(osx_arch_arm_64 TRUE)
      elseif("${osx_arch}" STREQUAL "ppc64" AND ppc_support)
        set(osx_arch_ppc64 TRUE)
      else()
        message(FATAL_ERROR "Invalid OS X arch name: ${osx_arch}")
      endif()
    endforeach()

    # Now add all the architectures in our normalized order
    if(osx_arch_ppc)
      list(APPEND ARCH ppc)
    endif()

    if(osx_arch_i386)
      list(APPEND ARCH i386)
    endif()

    if(osx_arch_x86_64)
      list(APPEND ARCH x86_64)
    endif()

    if(osx_arch_arm_64)
      list(APPEND ARCH arm64)
    endif()

    if(osx_arch_ppc64)
      list(APPEND ARCH ppc64)
    endif()
  else()
    file(WRITE "${CMAKE_BINARY_DIR}/arch.c" "${archdetect_c_code}")

    enable_language(C)

    # Detect the architecture in a rather creative way... This compiles a small C program which is a
    # series of ifdefs that selects a particular #error preprocessor directive whose message string
    # contains the target architecture. The program will always fail to compile (both because file
    # is not a valid C program, and obviously because of the presence of the #error preprocessor
    # directives... but by exploiting the preprocessor in this way, we can detect the correct target
    # architecture even when cross-compiling, since the program itself never needs to be run (only
    # the compiler/preprocessor)
    try_run(
      run_result_unused
      compile_result_unused
      "${CMAKE_BINARY_DIR}"
      "${CMAKE_BINARY_DIR}/arch.c"
      COMPILE_OUTPUT_VARIABLE RAWOUTPUT
      CMAKE_FLAGS CMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
    )

    # Parse the architecture name from the compiler output
    string(REGEX REPLACE ".*cmake_ARCH ([a-zA-Z0-9_]+).*" "\\1" ARCH "${RAWOUTPUT}")

    # If we are compiling with an unknown architecture this variable should already be set to
    # "unknown" but in the case that it's empty (i.e. due to a typo in the code), then set it to
    # unknown
    if(NOT ARCH)
      set(ARCH unknown)
    endif()
  endif()

  set(${output_var}
      "${ARCH}"
      PARENT_SCOPE
  )
endfunction()

# is_symbol_defined(<output_variable> <symbol>): Determines whether the current C compiler defines
# the given preprocessor symbol and stores the result in output_variable.
#
# This function utilizes `try_compile` and some preprocessor trickery to determine whether the given
# symbol is defined. For example, after calling `is_symbol_defined(is_64_bit __x86_64__)`, the
# variable `is_64_bit` will be set to `TRUE if compiling for a 64-bit environment.
function(is_symbol_defined output_variable symbol)
  enable_language(C)

  set(is_symbol_defined_code
      "
#if defined(${symbol})
int main() { return 0; }
#endif
"
  )

  file(WRITE "${CMAKE_BINARY_DIR}/is_symbol_defined.c" "${is_symbol_defined_code}")

  try_compile(
    is_symbol_defined_result "${CMAKE_BINARY_DIR}" "${CMAKE_BINARY_DIR}/is_symbol_defined.c"
  )

  if(is_symbol_defined_result)
    set(${output_variable}
        TRUE
        PARENT_SCOPE
    )
  else()
    set(${output_variable}
        FALSE
        PARENT_SCOPE
    )
  endif()
endfunction()
