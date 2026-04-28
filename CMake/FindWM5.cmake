# WildMagic5 (WM5) — MSVC v143 install layout under SDK/Library/v143/x64/{Debug,Release}.
# SuperBuild passes -DWM5_ROOT_DIR; otherwise a local default is used for standalone configures.
#
# Visual Studio multi-config: link Debug targets to *D.lib, all other configs to Release libs
# (RelWithDebInfo / MinSizeRel use the Release import libs — same CRT mix as Release WM5).

if(NOT WM5_ROOT_DIR)
  set(WM5_ROOT_DIR "C:/cr/CMakeExternals/Install/WM5/SDK")
endif()

set(WM5_INCLUDE_DIR "${WM5_ROOT_DIR}/Include")

set(_wm5_lib_root "${WM5_ROOT_DIR}/Library/v143/x64")
set(_wm5_dbg_core "${_wm5_lib_root}/Debug/Wm5CoreD.lib")
set(_wm5_rel_core "${_wm5_lib_root}/Release/Wm5Core.lib")
set(_wm5_dbg_math "${_wm5_lib_root}/Debug/Wm5MathematicsD.lib")
set(_wm5_rel_math "${_wm5_lib_root}/Release/Wm5Mathematics.lib")

# Per-config full paths (required when Debug libraries are absent for a Release-only WM5 install).
set(WM5_LIBRARIES
  "$<$<CONFIG:Debug>:${_wm5_dbg_core}>$<$<NOT:$<CONFIG:Debug>>:${_wm5_rel_core}>"
  "$<$<CONFIG:Debug>:${_wm5_dbg_math}>$<$<NOT:$<CONFIG:Debug>>:${_wm5_rel_math}>"
)

set(WM5_FOUND TRUE)
set(WM5_Wm5Core_FOUND TRUE)
set(WM5_Wm5Mathematics_FOUND TRUE)

# Unused by current CRIMSON targets but kept so optional COMPONENTS checks do not fail.
set(WM5_Wm5Physics_FOUND TRUE)
set(WM5_Wm5Imagics_FOUND TRUE)
set(WM5_Wm5WglGraphics_FOUND TRUE)

mark_as_advanced(WM5_ROOT_DIR WM5_INCLUDE_DIR WM5_LIBRARIES)
