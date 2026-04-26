MITK_CREATE_MODULE( VesselTree
  INCLUDE_DIRS Algorithms DataManagement Interactions IO Rendering IO/VesselForestData IO/vtkParametricSplineVesselPath
  DEPENDS MitkSceneSerializationBase CRIMSONUtils
  # CommonComputationalGeometry provides vtkParametricSpline / vtkKochanekSpline (VTK9+ modular targets).
  PACKAGE_DEPENDS PUBLIC Boost GSL VTK|CommonComputationalGeometry
)

set(_crimson_vessel_tree_tgt "")
if(TARGET MitkVesselTree)
  set(_crimson_vessel_tree_tgt MitkVesselTree)
elseif(TARGET VesselTree)
  set(_crimson_vessel_tree_tgt VesselTree)
endif()
if(_crimson_vessel_tree_tgt)
  find_package(Boost 1.60.0 REQUIRED COMPONENTS serialization)
  if(TARGET Boost::serialization)
    target_link_libraries(${_crimson_vessel_tree_tgt} PUBLIC Boost::serialization)
  endif()
endif()
# TinyXML 1.x: vcpkg installs tinyxml-config.cmake with unofficial-tinyxml::unofficial-tinyxml (correct Debug/Release IMPORTED_*).
# Manual find must use tinyxmld in debug/lib — plain "tinyxml" there is often the wrong CRT (LNK2038).
if(_crimson_vessel_tree_tgt)
  find_package(tinyxml CONFIG QUIET)
endif()
if(_crimson_vessel_tree_tgt AND TARGET unofficial-tinyxml::unofficial-tinyxml)
  target_link_libraries(${_crimson_vessel_tree_tgt} PUBLIC unofficial-tinyxml::unofficial-tinyxml)
else()
  find_path(CRIMSON_TINYXML_INCLUDE_DIR NAMES tinyxml.h PATHS ${CMAKE_PREFIX_PATH} PATH_SUFFIXES include)
  find_library(CRIMSON_TINYXML_LIBRARY_RELEASE NAMES tinyxml libtinyxml PATHS ${CMAKE_PREFIX_PATH} PATH_SUFFIXES lib)
  find_library(CRIMSON_TINYXML_LIBRARY_DEBUG NAMES tinyxmld libtinyxmld tinyxml libtinyxml PATHS ${CMAKE_PREFIX_PATH} PATH_SUFFIXES debug/lib)
  if(_crimson_vessel_tree_tgt AND CRIMSON_TINYXML_INCLUDE_DIR)
    target_include_directories(${_crimson_vessel_tree_tgt} SYSTEM PUBLIC ${CRIMSON_TINYXML_INCLUDE_DIR})
    if(MSVC AND CRIMSON_TINYXML_LIBRARY_DEBUG AND CRIMSON_TINYXML_LIBRARY_RELEASE)
      target_link_libraries(${_crimson_vessel_tree_tgt} PUBLIC
        $<$<CONFIG:Debug>:${CRIMSON_TINYXML_LIBRARY_DEBUG}>
        $<$<NOT:$<CONFIG:Debug>>:${CRIMSON_TINYXML_LIBRARY_RELEASE}>)
    elseif(MSVC AND CRIMSON_TINYXML_LIBRARY_RELEASE AND NOT CRIMSON_TINYXML_LIBRARY_DEBUG)
      message(WARNING "TinyXML: no Debug lib under installed/<triplet>/debug/lib (LNK2038 in Debug). Install: vcpkg install tinyxml:x64-windows")
      target_link_libraries(${_crimson_vessel_tree_tgt} PUBLIC ${CRIMSON_TINYXML_LIBRARY_RELEASE})
    elseif(CRIMSON_TINYXML_LIBRARY_DEBUG)
      target_link_libraries(${_crimson_vessel_tree_tgt} PUBLIC ${CRIMSON_TINYXML_LIBRARY_DEBUG})
    elseif(CRIMSON_TINYXML_LIBRARY_RELEASE)
      target_link_libraries(${_crimson_vessel_tree_tgt} PUBLIC ${CRIMSON_TINYXML_LIBRARY_RELEASE})
    endif()
  endif()
  if(_crimson_vessel_tree_tgt AND NOT TARGET unofficial-tinyxml::unofficial-tinyxml
      AND NOT (CRIMSON_TINYXML_INCLUDE_DIR AND (CRIMSON_TINYXML_LIBRARY_DEBUG OR CRIMSON_TINYXML_LIBRARY_RELEASE)))
    message(WARNING "TinyXML not found (VesselForest legacy XML IO). vcpkg: install tinyxml:x64-windows; reconfigure CMake.")
  endif()
endif()

IF( BUILD_TESTING )
  add_subdirectory(Testing)
ENDIF()
