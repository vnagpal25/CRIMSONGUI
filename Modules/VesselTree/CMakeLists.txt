MITK_CREATE_MODULE( VesselTree
  INCLUDE_DIRS Algorithms DataManagement Interactions IO Rendering IO/VesselForestData IO/vtkParametricSplineVesselPath
  DEPENDS MitkSceneSerializationBase CRIMSONUtils
  # CommonComputationalGeometry provides vtkParametricSpline / vtkKochanekSpline (VTK9+ modular targets).
  PACKAGE_DEPENDS PUBLIC Boost GSL VTK|CommonComputationalGeometry
)

# TinyXML 1.x (tinyxml.h). MSVC Debug needs the /MDd build from vcpkg (installed/<triplet>/debug/lib/ when using triplet x64-windows).
find_path(CRIMSON_TINYXML_INCLUDE_DIR NAMES tinyxml.h PATHS ${CMAKE_PREFIX_PATH} PATH_SUFFIXES include)
find_library(CRIMSON_TINYXML_LIBRARY_RELEASE NAMES tinyxml libtinyxml PATHS ${CMAKE_PREFIX_PATH} PATH_SUFFIXES lib)
find_library(CRIMSON_TINYXML_LIBRARY_DEBUG NAMES tinyxml libtinyxml PATHS ${CMAKE_PREFIX_PATH} PATH_SUFFIXES debug/lib)
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
if(_crimson_vessel_tree_tgt AND CRIMSON_TINYXML_INCLUDE_DIR)
  target_include_directories(${_crimson_vessel_tree_tgt} SYSTEM PUBLIC ${CRIMSON_TINYXML_INCLUDE_DIR})
  if(MSVC AND CRIMSON_TINYXML_LIBRARY_DEBUG AND CRIMSON_TINYXML_LIBRARY_RELEASE)
    target_link_libraries(${_crimson_vessel_tree_tgt} PUBLIC
      $<$<CONFIG:Debug>:${CRIMSON_TINYXML_LIBRARY_DEBUG}>
      $<$<NOT:$<CONFIG:Debug>>:${CRIMSON_TINYXML_LIBRARY_RELEASE}>)
  elseif(MSVC AND CRIMSON_TINYXML_LIBRARY_RELEASE AND NOT CRIMSON_TINYXML_LIBRARY_DEBUG)
    message(WARNING "TinyXML: no Debug import lib under installed/<triplet>/debug/lib (LNK2038 likely for Debug). Reinstall: vcpkg install tinyxml:x64-windows — vcpkg has no x64-windows-debug triplet; x64-windows ships both Debug and Release outputs.")
    target_link_libraries(${_crimson_vessel_tree_tgt} PUBLIC ${CRIMSON_TINYXML_LIBRARY_RELEASE})
  elseif(CRIMSON_TINYXML_LIBRARY_DEBUG)
    target_link_libraries(${_crimson_vessel_tree_tgt} PUBLIC ${CRIMSON_TINYXML_LIBRARY_DEBUG})
  elseif(CRIMSON_TINYXML_LIBRARY_RELEASE)
    target_link_libraries(${_crimson_vessel_tree_tgt} PUBLIC ${CRIMSON_TINYXML_LIBRARY_RELEASE})
  endif()
endif()
if(_crimson_vessel_tree_tgt AND NOT (CRIMSON_TINYXML_INCLUDE_DIR AND (CRIMSON_TINYXML_LIBRARY_DEBUG OR CRIMSON_TINYXML_LIBRARY_RELEASE)))
  message(WARNING "TinyXML not found (VesselForest legacy XML IO). Install: vcpkg install tinyxml:x64-windows (Debug libs live under installed/x64-windows/debug/lib/)")
endif()

IF( BUILD_TESTING )
  add_subdirectory(Testing)
ENDIF()
