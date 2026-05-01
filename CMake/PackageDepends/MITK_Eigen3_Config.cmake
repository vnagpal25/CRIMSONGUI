# vcpkg ships Eigen3Config.cmake (CONFIG). Bundled FindEigen3.cmake is MODULE-only and often
# misses header-only installs. Fall back to MODULE for Linux/macOS layouts without CONFIG.
find_package(Eigen3 CONFIG QUIET)
if(NOT Eigen3_FOUND)
  find_package(Eigen3 REQUIRED)
endif()
if(NOT EIGEN3_INCLUDE_DIR)
  if(DEFINED Eigen3_INCLUDE_DIRS)
    set(EIGEN3_INCLUDE_DIR "${Eigen3_INCLUDE_DIRS}")
  elseif(TARGET Eigen3::Eigen)
    get_target_property(EIGEN3_INCLUDE_DIR Eigen3::Eigen INTERFACE_INCLUDE_DIRECTORIES)
  endif()
endif()

list(PREPEND ALL_INCLUDE_DIRECTORIES ${EIGEN3_INCLUDE_DIR})
