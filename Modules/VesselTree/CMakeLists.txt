MITK_CREATE_MODULE( VesselTree
  INCLUDE_DIRS Algorithms DataManagement Interactions IO Rendering IO/VesselForestData IO/vtkParametricSplineVesselPath
  DEPENDS MitkSceneSerializationBase CRIMSONUtils
  PACKAGE_DEPENDS PUBLIC Boost GSL
)

# TinyXML 1.x (tinyxml.h): vcpkg install tinyxml:x64-windows (CMAKE_PREFIX_PATH must include installed/x64-windows).
find_path(CRIMSON_TINYXML_INCLUDE_DIR NAMES tinyxml.h PATHS ${CMAKE_PREFIX_PATH} PATH_SUFFIXES include)
find_library(CRIMSON_TINYXML_LIBRARY NAMES tinyxml libtinyxml)
set(_crimson_vessel_tree_tgt "")
if(TARGET MitkVesselTree)
  set(_crimson_vessel_tree_tgt MitkVesselTree)
elseif(TARGET VesselTree)
  set(_crimson_vessel_tree_tgt VesselTree)
endif()
if(_crimson_vessel_tree_tgt AND CRIMSON_TINYXML_INCLUDE_DIR AND CRIMSON_TINYXML_LIBRARY)
  target_include_directories(${_crimson_vessel_tree_tgt} SYSTEM PUBLIC ${CRIMSON_TINYXML_INCLUDE_DIR})
  target_link_libraries(${_crimson_vessel_tree_tgt} PUBLIC ${CRIMSON_TINYXML_LIBRARY})
elseif(_crimson_vessel_tree_tgt)
  message(WARNING "TinyXML not found (VesselForest legacy XML IO). Install: vcpkg install tinyxml:x64-windows")
endif()

IF( BUILD_TESTING )
  add_subdirectory(Testing)
ENDIF()
