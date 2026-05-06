# Building CRIMSON on Windows with MSVC and CMake

This is the modern Windows build path reflected by the current repository. The older `Documentation/` files describe the historical Visual Studio 2013 / Qt 5.7 workflow; use this file for the Qt 6 / MSVC 2022 dependency refresh branch.

## Known-Good Direction

Use a short, local path layout. The build writes absolute paths into generated files, and Windows path length still matters.

Recommended layout:

```text
C:/src/CRIMSONGUI-update
C:/crimson-build
C:/vcpkg
C:/v/MITK-sb/MITK-build
C:/v/vmtk-install
C:/Qt/6.x.x/msvc2022_64
```

The current checked-in defaults include developer-local paths such as `C:/Vansh_Files/Qt/6.11.0/msvc2022_64`. Treat those as examples and override them in CMake for your machine.

## Required Tools

- Windows 10 or Windows 11, 64-bit.
- Visual Studio 2022 with the Desktop development with C++ workload.
- MSVC v143 x64 toolset.
- Windows SDK installed through Visual Studio.
- CMake 3.30 or newer is recommended for this refreshed branch.
- Git for Windows.
- GNU `patch.exe`, usually available at `C:/Program Files/Git/usr/bin/patch.exe`.
- Qt 6 for MSVC 2022 x64.
- vcpkg with the `x64-windows` triplet.
- A prebuilt MITK tree, or enough time to build MITK through the superbuild.
- A VMTK install that exposes `VMTKConfig.cmake`.
- The CRIMSON presolver binary or zip expected by the existing superbuild.

The repository still has `cmake_minimum_required(VERSION 3.16)`, but the recent Boost/CMake policy work was done for modern CMake behavior, including CMake 3.30+ `CMP0167`. Use a current CMake unless you are deliberately reproducing an old build.

## Install Qt

Install a Qt 6 MSVC x64 kit. The kit must match the Visual Studio compiler family.

Example CMake value:

```text
Qt6_DIR=C:/Qt/6.11.0/msvc2022_64/lib/cmake/Qt6
```

Required Qt components are pulled by MITK/CRIMSON and include Widgets, Svg, Sql, Xml, UiTools, Help, LinguistTools, OpenGL, and optionally WebEngineWidgets.

Do not use a MinGW Qt kit with MSVC.

## Install vcpkg Dependencies

Install vcpkg at a short path such as `C:/vcpkg`.

Expected triplet:

```text
x64-windows
```

Packages used by the current patches include:

```powershell
vcpkg install cgal:x64-windows
vcpkg install eigen3:x64-windows
vcpkg install tinyxml:x64-windows
vcpkg install qwt:x64-windows
```

The current CMake logic assumes:

```text
C:/vcpkg/installed/x64-windows
C:/vcpkg/installed/x64-windows/share/cgal
C:/vcpkg/installed/x64-windows/share/eigen3
C:/vcpkg/installed/x64-windows/bin
```

If your vcpkg path differs, override the relevant CMake cache entries or update the local configure presets you use.

## MITK

The refreshed build supports using a prebuilt MITK tree.

Expected CMake value:

```text
EXTERNAL_MITK_DIR=C:/v/MITK-sb/MITK-build
```

This directory must contain `MITKConfig.cmake`.

Important notes:

- Use the same compiler architecture and runtime family as CRIMSON.
- Prefer Release or RelWithDebInfo MITK.
- Debug CRIMSON with Release MITK usually fails with MSVC `LNK2038` runtime and iterator-debug-level mismatches.
- MITK Python support is not friendly to Debug builds; the CMake scripts already steer MITK Python toward RelWithDebInfo when needed.

If MITK's external-project tree contains an `ep` folder, CRIMSON will prepend it to `CMAKE_PREFIX_PATH` through `CMake/CrimsonPreparePrebuiltMITK.cmake`.

## VMTK

VMTK may need to be built separately from MITK.

Expected CMake value:

```text
VMTK_DIR=C:/v/vmtk-install/lib
```

`VMTK_DIR` must be the folder that contains `VMTKConfig.cmake`. If the config lives somewhere else, point directly at that folder.

If configure fails with missing VMTK targets, verify:

- `VMTKConfig.cmake` exists under `VMTK_DIR`.
- `VMTK-Targets.cmake` is loadable from that install.
- VMTK was built shared and with the same compiler as CRIMSON/MITK.
- VTK used by VMTK matches the VTK used by MITK.

## Configure with CMake GUI

1. Open a Visual Studio 2022 Developer Command Prompt, or launch `cmake-gui` from an environment where MSVC and Git are available.
2. Set source directory to the repository root.
3. Set build directory to a short path, for example `C:/crimson-build`.
4. Click Configure.
5. Choose the generator:

```text
Visual Studio 17 2022
```

6. Choose platform:

```text
x64
```

7. Set or verify these cache entries:

```text
CRIMSON_USE_SUPERBUILD=ON
CMAKE_BUILD_TYPE=Release
Qt6_DIR=C:/Qt/6.11.0/msvc2022_64/lib/cmake/Qt6
EXTERNAL_MITK_DIR=C:/v/MITK-sb/MITK-build
VMTK_DIR=C:/v/vmtk-install/lib
CGAL_DIR=C:/vcpkg/installed/x64-windows/share/cgal
Eigen3_DIR=C:/vcpkg/installed/x64-windows/share/eigen3
CRIMSON_PATCH_EXECUTABLE=C:/Program Files/Git/usr/bin/patch.exe
```

8. Configure again until CMake has no blocking red entries.
9. Generate.

For Visual Studio generators, the real build configuration is selected inside Visual Studio or passed with `--config`. Keep it consistent with the dependency build type.

## Configure from Command Line

Example:

```powershell
cmake -S C:/src/CRIMSONGUI-update -B C:/crimson-build -G "Visual Studio 17 2022" -A x64 `
  -DCRIMSON_USE_SUPERBUILD=ON `
  -DQt6_DIR:PATH=C:/Qt/6.11.0/msvc2022_64/lib/cmake/Qt6 `
  -DEXTERNAL_MITK_DIR:PATH=C:/v/MITK-sb/MITK-build `
  -DVMTK_DIR:PATH=C:/v/vmtk-install/lib `
  -DCGAL_DIR:PATH=C:/vcpkg/installed/x64-windows/share/cgal `
  -DEigen3_DIR:PATH=C:/vcpkg/installed/x64-windows/share/eigen3 `
  -DCRIMSON_PATCH_EXECUTABLE:FILEPATH="C:/Program Files/Git/usr/bin/patch.exe"
```

## Build

Open the generated solution:

```text
C:/crimson-build/CRIMSON-superbuild.sln
```

Build `ALL_BUILD` in Release or RelWithDebInfo.

Command line equivalent:

```powershell
cmake --build C:/crimson-build --config Release
```

Do not build Debug against a Release-built MITK tree. The current Windows path expects Release or RelWithDebInfo unless every dependency, including MITK, CTK, ITK, VTK, VMTK, and vcpkg libraries, was also built as Debug. A Debug CRIMSON build against Release MITK commonly fails with missing Debug CTK import libraries and MSVC `LNK2038` runtime or `_ITERATOR_DEBUG_LEVEL` mismatches.

After the superbuild configures the inner CRIMSON project, the application solution is under:

```text
C:/crimson-build/CRIMSON-build/CRIMSON.sln
```

## Running CRIMSON

Use the generated launcher batch file from the inner build output:

```text
C:/crimson-build/CRIMSON-build/bin/startCRIMSON_release.bat
```

Use the launcher instead of starting the executable directly. It sets the runtime environment for CRIMSON, Qt, OCC, WM5, MITK, and vcpkg DLLs.

If the app starts in Visual Studio but not from Explorer, suspect missing runtime DLL paths first.

No extra runtime environment variables should be required for the default safe path. Recent runtime-hardening changes intentionally make the application conservative by default:

- Legacy CRIMSON mesh VTK payloads in `.ocms` files are skipped unless `CRIMSON_LOAD_LEGACY_MESH_DATA=1` is set.
- CRIMSON custom render mappers are disabled unless `CRIMSON_ENABLE_CUSTOM_MAPPERS=1` is set.

Those variables are diagnostic opt-ins, not normal setup steps. Leave them unset for the safest launch path with VTK 9.4.

## Debugging

For normal debugging, build RelWithDebInfo and attach Visual Studio to the running process launched by the generated batch file.

Avoid Debug unless MITK, ITK, VTK, VMTK, vcpkg dependencies, and CRIMSON are all Debug-compatible. A Release MITK with Debug CRIMSON is expected to fail at link time.

## Common Failures

### `patch.exe` not found

Install Git for Windows and set:

```text
CRIMSON_PATCH_EXECUTABLE=C:/Program Files/Git/usr/bin/patch.exe
```

### `find_package(Boost)` fails

Check whether MITK's `ep` tree contains Boost. If not, set:

```text
MITK_BOOST_ROOT=<path-to-boost-prefix>
```

The repository ships `CMake/FindBoost.cmake` to handle CMake 3.30+ fallback behavior.

### `find_package(VMTK)` fails

Set `VMTK_DIR` to the directory containing `VMTKConfig.cmake`. If MITK did not build VMTK, use a separate VMTK install.

### `vtkFiltersVerdict` or VTK component errors

This usually means an old VTK component name or mismatched VTK package is being used. The refreshed code expects VTK 9-style package components.

### OCC external build fails with `cl.exe` child-process or internal compiler errors

The OCC external is large enough that MSVC can occasionally fail in the nested build with errors such as `D8040`, `C1001`, "error creating or communicating with child process", or an MSBuild custom-build step exiting with code 1. This is an OCC build failure; the later `Could NOT find OCC` message from `CRIMSON-Configure` is a consequence of the missing OCC install, not the root cause.

By default the repository does not throttle the OCC build. If this failure reproduces on a machine, use the optional cache variables in `CMakeExternals/OCC.cmake` only as a diagnostic/workaround:

```text
OCC_BUILD_PARALLEL_LEVEL=1
OCC_NATIVE_BUILD_ARGS=/m:1 /p:CL_MPCount=1 /p:UseMultiToolTask=false
```

If these values were set while debugging and the build later becomes unexpectedly slow, clear them from `CMakeCache.txt` or set both cache entries back to empty.

### Scene load crashes in `vtkCommonCore-9.4.dll`

A crash shortly after loading a legacy scene mesh may show a stack mostly in `vtkCommonCore-9.4.dll`, `vtkRenderingCore-9.4.dll`, `vtkRenderingOpenGL2-9.4.dll`, and `MitkCore.dll`. The recent safe default is to skip legacy mesh VTK payloads and avoid CRIMSON custom mappers unless explicitly opted in.

For diagnostics only:

```powershell
$env:CRIMSON_LOAD_LEGACY_MESH_DATA = "1"
$env:CRIMSON_ENABLE_CUSTOM_MAPPERS = "1"
```

Set these one at a time when narrowing a renderer or legacy mesh issue. Do not use them for normal smoke testing.

### `LNK2038` in Debug

You are probably linking Debug CRIMSON against Release MITK/ITK or Release vcpkg libraries. Use Release/RelWithDebInfo, or rebuild the dependency stack in Debug.

### Missing DLLs at startup

Run through `startCRIMSON_release.bat`. Then check `CRIMSON_VCPKG_RUNTIME_PATH`, Qt `bin`, OCC `bin`, and MITK runtime paths.

### TinyXML or Qwt missing

Install the vcpkg packages and reconfigure:

```powershell
vcpkg install tinyxml:x64-windows qwt:x64-windows
```

## Files to Check When the Build Breaks

- `SuperBuild.cmake`: outer dependency orchestration and CRIMSON configure cache seeding.
- `CMake/CrimsonPreparePrebuiltMITK.cmake`: package paths for the inner CRIMSON configure pass.
- `CMake/PackageDepends/*`: MITK module package bridges.
- `CMakeExternals/MITK.cmake`: MITK superbuild options and Qt 6 selection.
- `Apps/CRIMSON/startCRIMSON.bat.in`: runtime environment.
- `Apps/NetlistEditor/cmake/Modules/FindQwt.cmake`: Qwt/vcpkg bridge.
- `Modules/VesselTree/CMakeLists.txt`: TinyXML/vcpkg bridge.
