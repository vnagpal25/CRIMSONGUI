# Building CRIMSON on Windows with MSVC and CMake

This is the modern Windows build path reflected by the current repository. The previous Visual Studio 2013 / Qt 5.7 build instructions were removed because they contradict this Qt 6 / MSVC 2022 dependency refresh branch.

## Known-Good Direction

Use a short, local path layout. The build writes absolute paths into generated files, and Windows path length still matters (max length of 140 chars)

Recommended layout:

```text
C:/src/CRIMSONGUI-update
C:/crimson-build
C:/vcpkg
C:/v/MITK-sb/MITK-build
C:/v/vmtk-install
C:/Qt/6.x.x/msvc2022_64
```

The current checked-in defaults include developer-local paths (listed below). Treat those as examples and override them in CMake for your machine.

## Checked-In Local Path Defaults

The codebase still contains several hardcoded Windows paths from the current refresh machine. They document the environment that was used to get the MSVC 2022 / Qt 6 build moving. You should override these cache entries in CMake instead of recreating the exact folder layout.

Active build-affecting defaults:

| Path | Where it appears | Meaning |
| --- | --- | --- |
| `C:/v/MITK-sb/MITK-build` | `SuperBuild.cmake`: `EXTERNAL_MITK_DIR` | Locally built MITK build directory containing `MITKConfig.cmake`. |
| `C:/Vansh_Files/Qt/6.11.0/msvc2022_64/lib/cmake/Qt6` | `SuperBuild.cmake`: `Qt6_DIR` | Qt 6 CMake package directory for the refresh machine. |
| `C:/Vansh_Files/Qt/6.11.0/msvc2022_64/lib/cmake` | `SuperBuild.cmake`: `CMAKE_PREFIX_PATH` entries | Qt CMake prefix derived from the same local Qt install. |
| `C:/v/vmtk-install/lib` | `SuperBuild.cmake`: `VMTK_DIR` | Separate VMTK install directory containing `VMTKConfig.cmake`. |
| `C:/vcpkg/installed/x64-windows` | `SuperBuild.cmake`, `CMake/CrimsonPreparePrebuiltMITK.cmake` | vcpkg install prefix used for package discovery and runtime DLLs. |
| `C:/vcpkg/installed/x64-windows/share/cgal` | `SuperBuild.cmake`, generated CRIMSON configure cache | `CGAL_DIR`. |
| `C:/vcpkg/installed/x64-windows/share/eigen3` | `SuperBuild.cmake`, generated CRIMSON configure cache | `Eigen3_DIR`. |
| `C:/vcpkg/installed/x64-windows/bin` | `CMake/CrimsonPreparePrebuiltMITK.cmake` | `CRIMSON_VCPKG_RUNTIME_PATH`. |
| `C:/v/presolver_20210317_1442.zip` | `CMakeExternals/presolver.cmake`: `presolver_url` | Local presolver zip used by the superbuild. |
| `C:/cr/CMakeExternals/Install/WM5/SDK` | `CMake/FindWM5.cmake`: fallback `WM5_ROOT_DIR` | Standalone WM5 fallback when the superbuild has not passed `WM5_ROOT_DIR`. |

Dormant, legacy, or diagnostic paths also present in source:

| Path | Where it appears | Meaning |
| --- | --- | --- |
| `C:\cb\CRIMSON-build` | comment in `SuperBuild.cmake` | Old/example generated inner build path. |
| `C:/cr/PythonModules` | comments in `CMakeExternals/PythonModules.cmake` | Example external-project source path. |
| `C:/cr/CMakeExternals/Source/flowsolver` | comments in `CMakeExternals/PythonModules.cmake` | Example flowsolver source path. |
| `C:/cr/CMakeExternals/Source/presolver/presolver.exe` | comments in `CMakeExternals/PythonModules.cmake` | Example presolver executable path. |
| `C:\crimson\rel\PythonModules` | comments in `CMakeExternals/PythonModules.cmake` | Historical example PythonModules output path. |
| `d:/1/ep/ep.zip` | `CMakeExternals/CMakeLists.txt` | Legacy/test external-project archive path. |
| `d:/kcl/` | `Modules/CGALVMTKMeshingKernel/ExtensionPoint/IMeshingKernel.cpp` | Disabled debug writer output path for temporary `.vtu`, `.vtp`, and `.off` mesh dumps. |
| `D:\Data\ownCloud\Work\CRIMSON Logo\CRIMSONBoundaryConditionToolboxLogo.svg` | `Apps/NetlistEditor/src/gui/images/CRIMSONBoundaryConditionToolboxLogo.svg` metadata | Inkscape source-file metadata embedded in an SVG asset. |
| `D:\Data\ownCloud\Work\CRIMSON Logo\CRIMSONBoundaryConditionToolboxLogo.svg.png` | `Apps/NetlistEditor/src/gui/images/CRIMSONBoundaryConditionToolboxLogo.svg` metadata | Inkscape export-file metadata embedded in an SVG asset. |
| `C:/local/boost_<version>`, `C:/boost`, `C:/boost/include`, `C:/boost/lib` | `CMake/FindBoost.cmake` | Generic upstream FindBoost Windows search fallbacks, not specific to this refresh machine. |

Recommended replacement pattern:

```powershell
cmake -S C:/src/CRIMSONGUI-update -B C:/crimson-build -G "Visual Studio 17 2022" -A x64 `
  -DQt6_DIR:PATH=<your-qt-prefix>/lib/cmake/Qt6 `
  -DEXTERNAL_MITK_DIR:PATH=<your-mitk-build> `
  -DVMTK_DIR:PATH=<your-vmtk-config-dir> `
  -DCGAL_DIR:PATH=<your-vcpkg>/installed/x64-windows/share/cgal `
  -DEigen3_DIR:PATH=<your-vcpkg>/installed/x64-windows/share/eigen3 `
  -Dpresolver_url:FILEPATH=<your-presolver-zip>
```

## Required Tools

- Windows 10/11, 64-bit.
- Visual Studio 2022 with the Desktop development with C++ workload.
- MSVC v143 x64 toolset.
- Windows SDK installed through Visual Studio.
- CMake 3.30 or newer
- Git for Windows
- GNU `patch.exe`, usually available at `C:/Program Files/Git/usr/bin/patch.exe`.
- Qt 6 for MSVC 2022 x64. (While building MITK)
- vcpkg with the `x64-windows` triplet.
- A locally built MITK tree. The current working CRIMSON configuration used MITK v2025.12.2 built separately from MITK's own superbuild instructions.
- A locally built and installed VMTK tree. The working layout used separate `vmtk-build` and `vmtk-install` folders.[^vmtk-build]
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

You should expect to build MITK yourself first. The refreshed CRIMSON build does not currently assume a downloadable MITK developer SDK is available; it points at an already-built MITK build tree. The working setup used MITK v2025.12.2, built from MITK's own superbuild, and then passed the resulting `MITK-build` directory to CRIMSON.

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
- MITK is a moving dependency. A newer MITK release may require dependency tweaks in CRIMSON, especially around Qt components, CTK/BlueBerry exports, VTK modules, Python wrapping, and generated package paths.

If MITK's external-project tree contains an `ep` folder, CRIMSON will prepend it to `CMAKE_PREFIX_PATH` through `CMake/CrimsonPreparePrebuiltMITK.cmake`.

## VMTK

VMTK also needs to be built separately, similar to MITK. The working setup used a dedicated VMTK source checkout, a `vmtk-build` directory, and a separate `vmtk-install` directory. Point CRIMSON at the installed VMTK CMake config, not merely at the source tree.

Expected CMake value:

```text
VMTK_DIR=C:/v/vmtk-install/lib
```

`VMTK_DIR` must be the folder that contains `VMTKConfig.cmake`. If the config lives somewhere else, point directly at that folder.

Recommended layout:

```text
C:/v/vmtk
C:/v/vmtk-build
C:/v/vmtk-install
```

Build VMTK in Release with the same MSVC x64 toolchain family used by MITK and CRIMSON. During the VMTK configure step, set its install prefix to the install folder, for example:

```text
CMAKE_INSTALL_PREFIX=C:/v/vmtk-install
```

After building and installing VMTK, set CRIMSON's `VMTK_DIR` to the installed folder that actually contains `VMTKConfig.cmake`. In the current local layout that is:

```text
VMTK_DIR=C:/v/vmtk-install/lib
```

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

Open the generated solution in Visual Studio 2022:

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


## Footnotes
MITK build: The working refresh used MITK v2025.12.2 and was built from the MITK build-instructions page recorded for that release: <https://docs.mitk.org/2025.12.2/BuildInstructionsPage.html>. MITK's public site identifies v2025.12.2 as a February 2026 hotfix release, while the accessible 2025-era MITK build instructions describe the superbuild requirements as Git, CMake 3.22 or newer, Qt 6.6 for Qt-based applications, and Doxygen plus GraphViz for documentation. Required Qt components include Qt 5 Compatibility Module, Qt State Machines, Qt WebEngine, and, for Qt 6.8 or later, Qt WebChannel and Qt Positioning. On Windows, MITK is built by configuring the MITK superbuild with a Visual Studio generator and building `BUILD_ALL`; the usable toolkit tree is the generated `MITK-build` directory containing `MITKConfig.cmake`. Sources checked: [MITK v2025.12.2 release page](https://www.mitk.org/) and [MITK build instructions](https://docs.mitk.org/2025.08/BuildInstructionsPage.html).

VMTK-build: VMTK's download/build page says to compile from source when a suitable binary package is not available, create a `vmtk-build` directory, run CMake against the VMTK source tree, and build with the platform compiler. It notes that the source build uses a CMake superbuild that downloads and builds VTK and ITK, can take a long time, and on Windows should use a Visual Studio amd64 generator with the Release configuration. The public instructions mention an automatic `vmtk-build/Install` output; for this CRIMSON refresh, use an explicit install prefix such as `C:/v/vmtk-install` so `VMTK_DIR` can point at the installed `VMTKConfig.cmake`. Source checked: [VMTK download/build instructions](https://www.vmtk.org/download/).
