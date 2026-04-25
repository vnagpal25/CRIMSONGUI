find_package(VMTK REQUIRED)

if(DEFINED VMTK_INCLUDE_DIRS)
  list(APPEND ALL_INCLUDE_DIRECTORIES ${VMTK_INCLUDE_DIRS})
endif()

# VMTKConfig.cmake loads imported targets from VMTK-Targets.cmake. Mirror the
# libraries from upstream VMTKUse.cmake without using deprecated link_libraries().
set(_crimson_vmtk_targets
  vtkvmtkCommon
  vtkvmtkComputationalGeometry
  vtkvmtkContrib
  vtkvmtkDifferentialGeometry
  vtkvmtkIO
  vtkvmtkITK
  vtkvmtkMisc
  vtkvmtkSegmentation
  nl
  tet
)
foreach(_t ${_crimson_vmtk_targets})
  if(TARGET ${_t})
    list(APPEND ALL_LIBRARIES ${_t})
  endif()
endforeach()

if(NOT ALL_LIBRARIES AND DEFINED VMTK_LIBRARIES AND VMTK_LIBRARIES)
  list(APPEND ALL_LIBRARIES ${VMTK_LIBRARIES})
endif()

if(NOT ALL_LIBRARIES)
  message(FATAL_ERROR "VMTK found but no libraries/targets were added. Check VMTK_DIR (folder containing VMTKConfig.cmake) and that VMTK-Targets.cmake is loadable.")
endif()
