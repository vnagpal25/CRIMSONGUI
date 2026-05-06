# Future Mac Porting Notes

The next major step is a macOS port. Based on the Windows dependency refresh, the Mac work should be treated as a dependency and packaging project first, then an application-code port second.

## Expected Blockers

### Qt 6 Kit and Architecture

macOS will force an explicit decision between Apple Silicon, Intel, or universal builds.

Expect blockers around:

- Qt 6 architecture matching the compiler and dependency stack.
- Qt WebEngine availability and codesigning behavior.
- `.app` bundle layout for Qt plugins, platforms, styles, image formats, and translations.
- Replacing Windows launcher batch-file assumptions with bundle/runtime path handling.

What to inspect:

- `Apps/CRIMSON/CMakeLists.txt`
- `CMake/crimsonFunctionCreateBlueBerryApplication.cmake`
- existing NetlistEditor Mac bundle snippets under `Apps/NetlistEditor/cmake`

### MITK and CTK Packaging

MITK/BlueBerry/CTK plugin discovery is likely to be the hardest Mac-specific part. On Windows the refresh had to patch imported CTK paths; on Mac the equivalent issue will probably be bundle-relative plugin and library discovery.

Expect blockers around:

- `@rpath`, `@loader_path`, and `@executable_path`.
- CTK plugin directories inside the `.app`.
- MITK runtime libraries not being copied into the bundle.
- BlueBerry plugin metadata being found at build time but not at app launch.

Likely work:

- Audit all install and bundle fixup code.
- Use `otool -L` to inspect unresolved dynamic libraries.
- Use CMake bundle utilities or explicit copy/fixup steps for MITK, CTK, Qt, VTK, VMTK, OCC, and vcpkg/Homebrew/MacPorts libraries.

### vcpkg Triplet Differences

Windows uses `x64-windows`. Mac will need a separate triplet and package strategy.

Expect blockers around:

- `arm64-osx` vs `x64-osx` vs universal builds.
- Packages that exist on Windows but build differently on Mac.
- Qwt, TinyXML, CGAL, Eigen, Boost, and OCC config locations.
- Runtime paths from vcpkg not matching current hardcoded `C:/vcpkg` assumptions.

Likely work:

- Remove hardcoded Windows vcpkg paths from `CMake/CrimsonPreparePrebuiltMITK.cmake`.
- Introduce cache variables for the vcpkg root and triplet.
- Make `CRIMSON_VCPKG_RUNTIME_PATH` platform-aware or replace it with bundle fixup logic on Mac.

### VTK and VMTK Consistency

The Windows work showed that VTK/VMTK version and build consistency is fragile. Mac will add ABI and runtime path constraints.

Expect blockers around:

- VMTK finding the same VTK that MITK uses.
- VTK libraries being shared and discoverable from the `.app`.
- VTK module names and imported targets varying by build.
- VMTK install layout differing from the Windows assumption that `VMTK_DIR` is under `lib`.

Likely work:

- Build MITK, VTK, and VMTK with the same compiler and architecture.
- Point `VMTK_DIR` exactly at the folder containing `VMTKConfig.cmake`.
- Audit `CMake/PackageDepends/MITK_VMTK_Config.cmake` for target names on Mac.

### OpenCASCADE

OCC path logic currently contains Windows-specific toolset and DLL assumptions. Mac will need separate include, library, framework, and runtime path handling.

Expect blockers around:

- `FindOCC.cmake` assuming `win64/<toolset>/lib`.
- OCC dependency libraries such as TBB, FreeType, and Tcl/Tk.
- `@rpath` fixes for OCC shared libraries.
- Potential conflict between CRIMSON's patched OCC external and vcpkg/Homebrew OCC.

Likely work:

- Extend `CMake/FindOCC.cmake` with Mac install layouts.
- Decide whether OCC comes from the superbuild, vcpkg, Homebrew, or a manual build.
- Keep one OCC source of truth. Mixing a superbuild OCC with package-manager transitive dependencies will be painful.

### Boost and CMake Policy Behavior

Boost discovery already required special handling on Windows. On Mac, this will show up again if MITK and CRIMSON see different Boost roots.

Expect blockers around:

- Boost from MITK `ep` vs vcpkg vs Homebrew.
- `BoostConfig.cmake` being found for one configure pass but not the other.
- CMake policy behavior changing between installed CMake versions.

Likely work:

- Keep `MITK_BOOST_ROOT` available.
- Verify both the outer superbuild and inner CRIMSON configure pass use the same Boost.

### Python and Presolver

The Windows build uses a prebuilt presolver and has MITK Python constraints. On Mac, these will probably need a fresh decision.

Expect blockers around:

- Python framework/library discovery.
- MITK Python wrapping support on the chosen architecture.
- The presolver binary format and Fortran runtime availability.
- Replacing Windows-only presolver paths and launcher assumptions.

Likely work:

- Decide whether presolver is packaged, rebuilt, or temporarily disabled on Mac.
- Keep Python paths out of hardcoded Windows directories.

### Source-Level Portability

Some source patches were Windows-driven but expose portability risks.

Expected source-level issues:

- Case-sensitive include paths on some Mac filesystems.
- Missing includes that MSVC tolerated differently.
- `byte` name conflicts moving from Windows to standard C++ headers in the other direction.
- Qt event, geometry, and file dialog behavior differing on Mac.
- VTK/MITK rendering assumptions around OpenGL contexts and Retina scaling.

Watch these areas first:

- `Modules/SolidKernel/*`
- `Modules/CGALVMTKMeshingKernel/*`
- `Modules/VesselTree/*`
- `Plugins/uk.ac.kcl.VascularModeling/*`
- `Plugins/uk.ac.kcl.SolverSetupView/*`

## Recommended Porting Order

1. Remove hardcoded Windows paths from CMake cache defaults while keeping the Windows defaults easy to set.
2. Get a clean configure with `CRIMSON_USE_SUPERBUILD=OFF` against an already-built MITK on Mac.
3. Make package discovery platform-aware for Qt6, Boost, CGAL, Eigen3, VMTK, VTK, OCC, Qwt, and TinyXML.
4. Build only the smallest module/plugin set needed to link the main app.
5. Fix compile errors module by module.
6. Launch from the build tree using explicit environment variables.
7. Only after launch works, build a proper `.app` bundle.
8. Add bundle fixup and codesigning.
9. Test rendering, meshing, solver setup, NetlistEditor, and file import/export workflows.

## Mac-Specific CMake Work to Do

Replace Windows-only assumptions with cache variables:

- vcpkg root and triplet
- Qt prefix
- MITK build root
- VMTK config directory
- OCC root
- runtime/library path setup

Add platform branches where needed:

- `WIN32`: batch files and DLL paths
- `APPLE`: `.app`, `@rpath`, bundle fixup, plugin copying
- `UNIX AND NOT APPLE`: Linux-style runtime paths

## Early Smoke Tests

Before trying to package the app, verify these:

```sh
cmake --build <build-dir> --config Release --target CRIMSON-Configure
cmake --build <build-dir>/CRIMSON-build --config Release
otool -L <path-to-CRIMSON-binary>
```

Then run from a terminal with explicit environment variables before attempting a double-clickable `.app`.

## The Big Lesson from Windows

Do not start the Mac port by chasing individual compile errors. First make the dependency graph boring and explicit. Most of the Windows pain came from hidden second-pass CMake state, mixed dependency roots, and runtime paths that were correct for build tools but wrong for launching the app.

