# Patches applied to G3D

Since the original G3D library has a larger set of dependencies which are not
of any use in World of Warcraft emulation, this version of the base library
has been modified accordingly.

The applied patches are added as .diff-files to the repository for future
reference.

## List of patches applied

| Date       | Filename                 | List of changes applied  |
| :--------- | :----------------------- | :----------------------- |
| 2010-08-27 | `G3D-v8.0_hotfix1.diff`  | remove dependency on zip/z11 libraries, add support for 64-bit arch |
| 2012-01-14 | `G3D-v8.0_hotfix2.diff`  | fix typo in isNaN(float x) |
| 2012-08-26 | `G3D-v8.0_hotfix3.diff`  | fix compilation on Fedora Linux |
| 2012-11-09 | `G3D-v8.0_hotfix4.diff`  | fix compilation on OSX |
| 2013-02-27 | `G3D-v8.0_hotfix5.diff`  | fix compilation in cygwin environments |
| 2013-03-08 | `G3D-v8.0_hotfix6.diff`  | fix compilation in mingw |
| 2013-08-31 | `G3D-v8.0_hotfix7.diff`  | fix typo in Matrix4 == operator |
| 2013-09-01 | `G3D-v8.0_hotfix8.diff`  | fix typo in Vector3int32 += operator |
| 2014-06-01 | `G3D-v8.0_hotfix9.diff`  | only VS < 10 don't ship inttypes.h |
| 2014-08-22 | `G3D-v9.0 hotfix1.diff`  | updated to G3D9, reapplied previous patches and removed unneeded changes |
| 2014-08-23 | `G3D-v9.0 hotfix2.diff`  | fix some -Wconversion warnings |
| 2015-06-28 | `G3D-v9.0 hotfix3.diff`  | fix some warnings |
| 2015-07-02 | `G3D-v9.0 hotfix4.diff`  | backport G3D10 fix |
| 2015-07-31 | `G3D-v9.0 hotfix5.diff`  | fix MSVC 2015 warning: dep/g3dlite/include/G3D/Quat.h(352): warning C4458: declaration of 'x' hides class member |
| 2015-11-04 | `G3D-v9.0 hotfix6.diff`  | fix adding std::shared_ptr, std::weak_ptr, std::dynamic_pointer_cast, std::static_pointer_cast and std::enable_shared_from_this to global namespace |
| 2016-10-10 | `G3D-v9.0 hotfix7.diff`  | fix warning on clang 3.8 backported from G3D 10 |
| 2017-03-07 | `G3D-v9.0 hotfix8.diff`  | fix MSVC 2017 build |
| 2020-07-13 | `G3D-v9.0 hotfix9.diff`  | warning fixes |
| 2017-11-11 | `G3D-v9.0 hotfix10.diff` | static analysis issue fix |
| 2017-11-11 | `G3D-v9.0 hotfix11.diff` | static analysis issue fix |
| 2020-02-23 | `G3D-v9.0 hotfix12.diff` | ARM64 build fix |
| 2020-08-04 | `G3D-v9.0 hotfix13.diff` | MSVC build fix with /permissive- |
| 2022-01-16 | `G3D-v9.0 hotfix14.diff` | warning fixes |
| 2022-02-11 | `G3D-v9.0 hotfix15.diff` | purge Windows.h includes from public headers |
| 2022-02-11 | `G3D-v9.0 hotfix16.diff` | Fix *BSD compile errors |
| 2023-03-25 | `G3D-v9.0 hotfix17.diff` | Fix macOS compile errors |
| 2024-02-17 | `G3D-v9.0 hotfix18.diff` | ARM64 support on MSVC |
| 2024-06-09 | `G3D-v9.0 hotfix19.diff` | clang build fix on windows |
