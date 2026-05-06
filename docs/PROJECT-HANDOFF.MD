# Project Handoff

This folder documents the current dependency refresh and build state after the recent 90-commit modernization pass.

## Start Here

- `DEPENDENCY-REFRESH.MD`: narrative of the major blockers, patches, and dependency lessons.
- `BUILD-WINDOWS-MSVC-CMAKE.MD`: practical Windows build instructions for MSVC, CMake, Qt 6, vcpkg, MITK, and VMTK.
- `FUTURE-MAC-PORTING-NOTES.MD`: expected blockers and recommended order for the next macOS port.

## Current State in One Page

The project is no longer just the old Visual Studio 2013 / Qt 5.7 superbuild. The current branch has been pushed toward:

- Visual Studio 2022 / MSVC v143
- Qt 6
- CMake 3.30+ behavior
- prebuilt MITK support
- separate VMTK support
- vcpkg for CGAL, Eigen3, TinyXML, Qwt, and runtime DLLs
- VTK 9-style components and APIs
- conservative runtime rendering defaults for legacy scenes under VTK 9.4

The old docs under `Documentation/` are still useful as historical context, especially for superbuild concepts and old CRIMSON workflow expectations, but they do not describe the refreshed dependency stack accurately.

## Latest Runtime Decision

Legacy scene loading exposed a VTK 9.4 crash in the rendering path after `.ocms` mesh data was encountered. The current default is intentionally conservative:

- Legacy CRIMSON mesh VTK payloads are skipped by default.
- A placeholder mesh object is still returned so MITK scene loading does not treat the scene as corrupted.
- CRIMSON custom mappers are disabled by default for mesh, solid, PCMRI, and vessel-tree data.

There are no extra config steps for normal users. The environment variables `CRIMSON_LOAD_LEGACY_MESH_DATA=1`, `CRIMSON_OMIT_SKIPPED_LEGACY_MESH_NODE=1`, and `CRIMSON_ENABLE_CUSTOM_MAPPERS=1` are diagnostic opt-ins only.

## High-Risk Areas

- CMake package discovery across the outer superbuild and inner `CRIMSON-Configure` project.
- Runtime DLL/path setup for generated launchers and Visual Studio debugging.
- QtPropertyBrowser and CTK patches.
- VTK/VMTK target names and API drift.
- VTK 9.4 rendering of CRIMSON custom mappers and legacy `.ocms` mesh payloads.
- Eigen include order.
- Debug/Release mixing under MSVC.
- Mac bundle/runtime path work.

## Useful History Commands

Review the modernization pass:

```sh
git log --oneline -n 90
git diff --stat HEAD~90..HEAD
git diff --name-only HEAD~90..HEAD
```

Inspect dependency-heavy commits:

```sh
git log --oneline -- CMakeLists.txt SuperBuild.cmake CMake/CrimsonPreparePrebuiltMITK.cmake CMakeExternals/MITK.cmake
git log --oneline -- CMakeExternals/QtPropertyBrowser.cmake CMakeExternals/QtPropertyBrowser_qt6_patch.cmake Apps/NetlistEditor
git log --oneline -- Modules/CGALVMTKMeshingKernel Modules/SolidKernel Modules/VesselTree Modules/PCMRIKernel
```

## Before Making More Dependency Changes

1. Record the exact tool versions and install paths being used.
2. Change one dependency boundary at a time.
3. Configure from a clean build directory after package layout changes.
4. Test the generated launcher, not just the Visual Studio debugger.
5. Keep Debug/Release configurations consistent across MITK, VTK, VMTK, vcpkg, and CRIMSON.
6. If a crash appears only after the workbench starts rendering, first compare with the safe mapper defaults before changing file readers.
