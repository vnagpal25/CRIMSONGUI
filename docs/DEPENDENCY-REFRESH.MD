# Dependency Refresh Notes

This document summarizes the recent dependency refresh work across the latest 90 commits. It is intentionally written as a handoff record: what blocked the build, why it blocked, what was patched, and what a future maintainer should watch before changing the dependency graph again.

## Scope

The refresh moved the project away from the old Visual Studio 2013 / Qt 5.7 era assumptions and toward a Windows MSVC 2022, Qt 6, CMake, prebuilt-MITK, and vcpkg-based workflow.

The heaviest churn was in:

- `SuperBuild.cmake`
- `CMakeLists.txt`
- `CMake/CrimsonPreparePrebuiltMITK.cmake`
- `CMake/PackageDepends/*`
- `CMakeExternals/MITK.cmake`
- `CMakeExternals/QtPropertyBrowser*.cmake`
- `Apps/NetlistEditor/*`
- `Modules/CGALVMTKMeshingKernel/*`
- `Modules/PCMRIKernel/*`
- `Modules/SolidKernel/*`
- `Modules/VesselTree/*`
- Solver setup and vascular modeling plugins that use MITK, VTK, and Qt APIs directly

## Blocker 1: Qt 5 to Qt 6

### Encountered Issues

CRIMSON had Qt 5 assumptions in both build scripts and source code. The refresh had to update CMake package discovery, link targets, deprecated Qt APIs, and vendored QtPropertyBrowser code.

Common symptoms:

- `find_package(Qt5...)` no longer matched the dependency stack.
- QtPropertyBrowser failed on removed or renamed Qt 5 APIs.
- `QRegExp` usage failed or required `Core5Compat`.
- Legacy Qt link variables expanded to bare library names and caused MSVC link failures.
- UI code compiled against older Qt types such as `QStyleOptionViewItemV2/V3`.

### Patches made

- Switched CRIMSON and MITK configuration toward Qt 6:
  - `CMakeLists.txt`
  - `SuperBuild.cmake`
  - `CMakeExternals/MITK.cmake`
  - `Apps/CRIMSON/CMakeLists.txt`
  - `Apps/NetlistEditor/src/CMakeLists.txt`
- Added `CMake/PackageDepends/MITK_Qt6_Config.cmake`.
- Added a Qt 6 patch pipeline for QtPropertyBrowser:
  - `CMakeExternals/QtPropertyBrowser.cmake`
  - `CMakeExternals/QtPropertyBrowser_qt6_patch.cmake`
- Replaced `QRegExp` with `QRegularExpression` in NetlistEditor and solver Python plugin code.
- Removed dependence on `Core5Compat` after replacing Qt 5 regex APIs.
- Linked Qt through `Qt6::*` imported targets in NetlistEditor instead of legacy variables.
- Patched CTK and vendored QtPropertyBrowser copies under `Patch/CTK` and `Patch/commontk_QtPropertyBrowser`.

## Blocker 2: VTK 9 and VMTK as a Separate Build

### What broke

The older build expected VTK and VMTK to appear through MITK-era package variables. Newer VTK uses more modular component names and imported targets, and VMTK may not exist inside the MITK external project tree. In practice, CRIMSON needed to find a separate VMTK install and pass it through the CRIMSON configure step.

Common symptoms:

- `find_package(VMTK)` failed during CRIMSON configure.
- VTK package components had old names, such as `vtkFiltersVerdict` instead of `FiltersVerdict`.
- VTK 9 API changes broke mesh and solid modules.
- VTK types required explicit includes, for example `vtkDataArray.h`.

### Patches made

- Added VMTK package configuration:
  - `CMake/PackageDepends/MITK_VMTK_Config.cmake`
- Added VTK package configuration:
  - `CMake/PackageDepends/MITK_VTK_Config.cmake`
- Updated `SuperBuild.cmake` to pass `VMTK_DIR` and seed `CMAKE_PREFIX_PATH` through an initial cache file.
- Added logic for separately built VMTK:
  - `VMTK_DIR` defaults to a local install path and should point at the folder containing `VMTKConfig.cmake`.
  - The VMTK install root is appended to the CRIMSON configure prefix path.
- Updated VTK component names in `Modules/CGALVMTKMeshingKernel/CMakeLists.txt`.
- Reworked VTK 9 API usage in meshing, solid, vessel tree, and solver setup code.
- Added `PlanarFigureMITKCompat.h` in vascular modeling and solver setup plugins to isolate MITK/planar-figure API drift.

## Blocker 2a: Legacy Mesh Payloads and VTK 9.4 Rendering

### What broke

Legacy CRIMSON scene files can contain serialized mesh VTK payloads that were written for an older VTK/MITK stack. With VTK 9.4, loading a scene could reach the workbench and then crash during rendering with a stack in `vtkCommonCore-9.4.dll`, `vtkRenderingCore-9.4.dll`, `vtkRenderingOpenGL2-9.4.dll`, and `MitkCore.dll`.

Important observations from the runtime investigation:

- The crash happened after the `.ocms` reader logged successfully, not during the first file open call.
- Returning no mesh node for a skipped payload avoided the reader path but made scene loading report a corrupted file, because MITK scene deserialization expected the object to exist.
- Disabling all CRIMSON custom mappers avoided the crash, which points at custom render mapper behavior rather than the image reader or generic MITK scene reader.

### Patches made

- `MeshDataIO.cpp` now skips legacy CRIMSON mesh VTK payloads by default. The diagnostic opt-in is `CRIMSON_LOAD_LEGACY_MESH_DATA=1`.
- Skipped legacy meshes keep a placeholder `MeshData` object so scene loading can continue without reporting the scene as corrupted.
- Placeholder VTK objects are initialized with empty points/cells and expected face-id arrays instead of leaving partially initialized data structures.
- `MeshData` tracks whether a legacy payload was skipped.
- Custom CRIMSON render mappers are disabled by default across mesh, solid, PCMRI, and vessel-tree object factories. The diagnostic opt-in is `CRIMSON_ENABLE_CUSTOM_MAPPERS=1`.

The default user path should not require any new environment variables. The environment variables above are for debugging older scene rendering behavior only.


## Blocker 3: vcpkg Integration

### What changed

The refresh uses vcpkg as the practical source for several dependencies that were otherwise brittle under modern Windows CMake:

- CGAL
- Eigen3
- TinyXML
- Qwt
- runtime DLLs used by those packages

The working triplet in the patches is `x64-windows`.

### Patches made

- Added vcpkg prefix handling in `SuperBuild.cmake`.
- Added `CMake/CrimsonPreparePrebuiltMITK.cmake` to prepare the non-superbuild CRIMSON configure pass.
- Forced these package directories for the Windows path:
  - `CGAL_DIR=C:/vcpkg/installed/x64-windows/share/cgal`
  - `Eigen3_DIR=C:/vcpkg/installed/x64-windows/share/eigen3`
  - `CRIMSON_VCPKG_RUNTIME_PATH=C:/vcpkg/installed/x64-windows/bin`
- Added package-dependency bridge files:
  - `CMake/PackageDepends/MITK_CGAL_Config.cmake`
  - `CMake/PackageDepends/MITK_Eigen3_Config.cmake`
- Added Qwt discovery for NetlistEditor:
  - `Apps/NetlistEditor/cmake/Modules/FindQwt.cmake`
- Added TinyXML detection and imported target handling:
  - `Modules/VesselTree/CMakeLists.txt`
- Updated generated Visual Studio user files and launcher batch script to include vcpkg runtime DLL paths:
  - `Apps/CRIMSON/startCRIMSON.bat.in`
  - `Apps/CRIMSON/CRIMSON.vcxproj.user.in`
  - `CMake/CRIMSON.vcxproj.user.in`

## Blocker 4: Boost and New CMake Policies

### What broke

Modern CMake changed Boost discovery behavior, especially around policy `CMP0167`. MITK and CRIMSON both need Boost, but prebuilt MITK layouts vary: some expose `BoostConfig.cmake`, while some look like older `b2` installs.

Common symptoms:

- `find_package(Boost)` could not find Boost under CMake 3.30+.
- `Boost_ROOT` and `BOOST_ROOT` were treated differently depending on policy state.
- CRIMSON configure did not see the same Boost tree used by MITK.

### Patches made

- Added `CMake/FindBoost.cmake`, copied from a CMake version that still ships module-mode Boost discovery.
- Added `CMake/PackageDepends/MITK_Boost_Config.cmake`.
- Added `MITK_BOOST_ROOT` handling in both `SuperBuild.cmake` and `CMake/CrimsonPreparePrebuiltMITK.cmake`.
- Set relevant CMake policies in `CMakeLists.txt` and `CMakeExternals/MITK.cmake`, especially:
  - `CMP0074`
  - `CMP0144`
  - `CMP0167`


## Blocker 5: Eigen Header Version Mixing

### What broke

Eigen was present from more than one place: vcpkg, MITK/ITK vendored includes, and direct module includes. MSVC then saw incompatible macros and deprecations depending on include order.

Common symptoms:

- `EIGEN_NOEXCEPT` or deprecation macro errors under MSVC.
- Mixed Eigen include versions across PCMRI and CGAL/VMTK modules.
- Compile failures in templated linear solver code.

### Patches made

- Added `Modules/PCMRIKernel/ExtensionPoint/internal/EigenCompat.h`.
- Updated PCMRI headers and linear solver code to include the compatibility wrapper.
- Prepended Eigen include directories in `CMake/PackageDepends/MITK_Eigen3_Config.cmake`.
- Added detection of ITK vendored Eigen in `CMake/CrimsonPreparePrebuiltMITK.cmake`.
- Removed or narrowed direct Eigen dependencies from modules where MITK already provides the path.


## Blocker 6: MSVC 2022 Toolset and Runtime Behavior

### What broke

The original project had old Visual Studio assumptions. Modern MSVC needed path detection, large object file flags, static flag cleanup, and runtime path propagation.

Common symptoms:

- `/bigobj` errors in large generated/template-heavy translation units.
- `-static` leaking into MSVC command lines.
- OCC and WM5 lookup failing because toolset names ended at `vc12`.
- Debug CRIMSON linking against Release MITK caused `LNK2038`.
- Missing DLLs at launch even after a successful build.

### Patches made

- Added `/bigobj` and `/MP` handling in `SuperBuild.cmake`, `CMakeLists.txt`, and module CMake files.
- Normalized semicolon-separated CMake flags before forwarding them through `ExternalProject_Add`.
- Removed GCC-style `-static` for MSVC in NetlistEditor.
- Updated WM5 and OCC find logic for VS2019/VS2022-style toolsets:
  - `CMake/FindWM5.cmake`
  - `CMakeExternals/WM5.cmake`
  - `CMake/FindOCC.cmake`
  - `CMakeExternals/OCC.cmake`
- Added warnings for Debug CRIMSON with Release-built MITK.
- Updated start/debug environments to include Qt, OCC, WM5, vcpkg, and CRIMSON runtime paths.

### Recent OCC/MSVC note

The OCC external can fail under MSVC 2022 with child-process or internal compiler errors during its nested build. When that happens, the later `CRIMSON-Configure` error that OCC cannot be found is usually secondary: OCC did not finish installing.

Optional cache variables exist for diagnosing this, but they intentionally default to empty:

- `OCC_BUILD_PARALLEL_LEVEL`
- `OCC_NATIVE_BUILD_ARGS`

Only set them when reproducing an OCC compiler failure. Leaving throttling values in the cache can make later builds much slower without helping normal successful builds.

## Blocker 7: Prebuilt MITK Export Quirks

### What broke

CRIMSON configures itself in a second CMake pass. The MITK setup that exists during the superbuild pass is not automatically replayed during that second pass. Prebuilt MITK exports also sometimes contain bad CTK imported paths.

Common symptoms:

- CRIMSON configure could not find dependency packages even though MITK built successfully.
- CTK targets pointed at duplicated `CTK-build/CTK-build` paths.
- Imported libraries pointed under `bin/<Config>` while MSVC import libraries lived under `lib/<Config>`.

### Patches made

- Added `CMake/CrimsonPreparePrebuiltMITK.cmake`.
- Called `crimson_prepare_prebuilt_mitk()` before `find_package(MITK ...)`.
- Added `crimson_fix_prebuilt_mitk_ctk_imported_paths()` after MITK discovery.
- Used an initial cache file from `SuperBuild.cmake` to pass multi-entry `CMAKE_PREFIX_PATH` reliably into the CRIMSON configure project.


## Patch Inventory

Dependency and build-system patches added or heavily reworked:

- `CMake/CrimsonPreparePrebuiltMITK.cmake`
- `CMake/FindBoost.cmake`
- `CMake/FindOCC.cmake`
- `CMake/FindWM5.cmake`
- `CMake/PackageDepends/MITK_Boost_Config.cmake`
- `CMake/PackageDepends/MITK_CGAL_Config.cmake`
- `CMake/PackageDepends/MITK_Eigen3_Config.cmake`
- `CMake/PackageDepends/MITK_Qt6_Config.cmake`
- `CMake/PackageDepends/MITK_VMTK_Config.cmake`
- `CMake/PackageDepends/MITK_VTK_Config.cmake`
- `CMakeExternals/MITK.cmake`
- `CMakeExternals/OCC.cmake`
- `CMakeExternals/OCC_6.9.1.patch`
- `CMakeExternals/QtPropertyBrowser.cmake`
- `CMakeExternals/QtPropertyBrowser_qt6_patch.cmake`
- `CMakeExternals/WM5.cmake`
- `Apps/NetlistEditor/cmake/Modules/FindQwt.cmake`
- `Modules/PCMRIKernel/ExtensionPoint/internal/EigenCompat.h`
- `Plugins/uk.ac.kcl.SolverSetupView/src/internal/PlanarFigureMITKCompat.h`
- `Plugins/uk.ac.kcl.VascularModeling/src/internal/PlanarFigureMITKCompat.h`

## Practical Lessons

- The dependency refresh is not one patch. It is a chain: Qt 6 forces MITK/CTK/QtPropertyBrowser changes; MITK changes expose VTK/VMTK changes; VTK/VMTK changes expose Eigen and package-discovery issues; vcpkg then fixes discovery but creates runtime DLL and configuration consistency requirements.
- Build configuration must be treated as source code. The fragile parts are include order, imported target properties, generated user files, and initial cache propagation.
- Avoid "latest everything" upgrades. Pin toolchains and dependency roots long enough to get a known-good build, then upgrade one dependency boundary at a time.
- When a CMake error says a package is missing, check the second CRIMSON configure pass, not just the outer superbuild.
- Treat runtime rendering changes the same way as dependency changes: prefer safe defaults, add explicit diagnostic opt-ins, and avoid making users set environment variables for the normal path.
