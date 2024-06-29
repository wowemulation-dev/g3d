# G3D Innovation Engine Code: base library

This project provides a port of the [G3D Innovation Engine Code][], specifically
the base library, which provides common primitives used in 3D rendering.

G3D is heavily used in World of Warcraft emulation, we opted for making a full
conversion to the [CMake][] build system, allowing for integration into projects
built with CMake via plain [FetchContent][] downloads or the [CPM.cmake][]
dependency manager.

## Available versions

For projects like (C)MaNGOS, TrinityCore, AzerothCore we provide a `v9` branch
providing a ported version of the latest version `9.x` code. For newer projects
such as WoW Emulation itself, we maintain a `v10` branch containing updated code
using the latest version `10.x` code.

The default branch is `v9` until the majority of World of Warcraft servers have
migrated to version 10 of the G3D base library.

[G3D Innovation Engine Code]: https://sourceforge.net/projects/g3d/
[CMake]: https://cmake.org/
[FetchContent]: https://cmake.org/cmake/help/latest/module/FetchContent.html
[CPM.cmake]: https://github.com/cpm-cmake/CPM.cmake
