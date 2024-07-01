# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright 2024, WoW Emulation and contributors

# ---- Add source files ----

# Note: globbing sources is considered bad practice as CMake's generators may not detect new files
# automatically. Keep that in mind when changing files, or explicitly mention them here.
file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")

set(sources
  source/AABox.cpp
  source/Any.cpp
  source/AnyTableReader.cpp
  source/BinaryFormat.cpp
  source/BinaryInput.cpp
  source/BinaryOutput.cpp
  source/Box.cpp
  source/Capsule.cpp
  source/CollisionDetection.cpp
  source/CoordinateFrame.cpp
  source/Crypto.cpp
  source/Cylinder.cpp
  source/debugAssert.cpp
  source/FileSystem.cpp
  source/fileutils.cpp
  source/format.cpp
  source/g3dfnmatch.cpp
  source/g3dmath.cpp
  source/GThread.cpp
  source/Line.cpp
  source/LineSegment.cpp
  source/Log.cpp
  source/Matrix3.cpp
  source/Matrix4.cpp
  source/MemoryManager.cpp
  source/PhysicsFrame.cpp
  source/Plane.cpp
  source/prompt.cpp
  source/Quat.cpp
  source/Random.cpp
  source/Ray.cpp
  source/RegistryUtil.cpp
  source/Sphere.cpp
  source/stringutils.cpp
  source/System.cpp
  source/TextInput.cpp
  source/TextOutput.cpp
  source/Triangle.cpp
  source/uint128.cpp
  source/UprightFrame.cpp
  source/Vector2.cpp
  source/Vector3.cpp
  source/Vector4.cpp
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  file(GLOB G3D_COCOA
  	"source/*.m"
  )
  set (sources ${sources} ${G3D_COCOA})
endif()

# ---- Create library ----

# Note: for header-only libraries change all PUBLIC flags to INTERFACE and create an interface
# target: add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME} ${headers} ${sources})
add_library(${PROJECT_NAME}::Base ALIAS ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 11)

# being a cross-platform target, we enforce standards conformance on MSVC
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")

if(G3DLIB_USE_ZIP)
    target_compile_definitions(${PROJECT_NAME} PUBLIC _HAVE_ZIP)
endif()

# Link dependencies
if(G3DLIB_USE_ZIP)
  target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB zip)
else()
  target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB)
endif()


target_include_directories(
  ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                         $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)

# ---- Create an installable target ----
# this allows users to install and find the library via `find_package()`.

# the location where the project's version header will be placed should match the project's regular
# header paths
string(TOLOWER version.h VERSION_HEADER_LOCATION)

packageProject(
  NAME ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION}
  NAMESPACE ${PROJECT_NAME}
  BINARY_DIR ${PROJECT_BINARY_DIR}
  INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/G3D
  INCLUDE_DESTINATION include/${PROJECT_NAME}
  VERSION_HEADER "${VERSION_HEADER_LOCATION}"
  DISABLE_VERSION_SUFFIX YES
  DEPENDENCIES "ZLIB;zip"
)
