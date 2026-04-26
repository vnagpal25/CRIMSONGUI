# FindQwt: bridge Qwt to the legacy variables used by Apps/NetlistEditor/src/CMakeLists.txt
# (Qwt_INCLUDE_DIRS, Qwt_LIBRARIES, Qwt5_INCLUDE_DIR for packaging paths).
#
# vcpkg's unofficial-qwt config is sometimes broken (empty prefix -> "//include", missing
# IMPORTED_LOCATION). When the imported target is unusable, fall back to find_path/find_library
# against CMAKE_PREFIX_PATH (vcpkg installed/<triplet> must be on that path).
#
# Legacy Qwt 5 + Qt4 remains available via FindQwt5.cmake when Qt4 is present.
#
# External: vcpkg install qwt:<triplet>; ensure installed/<triplet> is on CMAKE_PREFIX_PATH.

function(_crimson_qwt_vcpkg_target_ok tgt ok_var)
  set(${ok_var} 0 PARENT_SCOPE)
  if(NOT TARGET "${tgt}")
    return()
  endif()
  get_target_property(_inc "${tgt}" INTERFACE_INCLUDE_DIRECTORIES)
  if(NOT _inc OR _inc STREQUAL "_inc-NOTFOUND")
    return()
  endif()
  string(FIND "${_inc}" "$<" _genex)
  if(_genex LESS 0)
    foreach(_d IN LISTS _inc)
      if(_d STREQUAL "" OR _d STREQUAL "//include" OR NOT EXISTS "${_d}")
        return()
      endif()
    endforeach()
  endif()
  get_target_property(_loc "${tgt}" IMPORTED_LOCATION)
  get_target_property(_locs "${tgt}" IMPORTED_CONFIGURATIONS)
  set(_have_loc 0)
  if(_loc AND NOT _loc STREQUAL "_loc-NOTFOUND")
    set(_have_loc 1)
  endif()
  if(NOT _have_loc AND _locs AND NOT _locs STREQUAL "_locs-NOTFOUND")
    foreach(_cfg IN LISTS _locs)
      string(TOUPPER "${_cfg}" _CFGU)
      get_target_property(_lc "${tgt}" IMPORTED_LOCATION_${_CFGU})
      if(_lc AND NOT _lc STREQUAL "_lc-NOTFOUND")
        set(_have_loc 1)
        break()
      endif()
    endforeach()
  endif()
  if(NOT _have_loc)
    return()
  endif()
  set(${ok_var} 1 PARENT_SCOPE)
endfunction()

find_package(unofficial-qwt CONFIG QUIET)
_crimson_qwt_vcpkg_target_ok(unofficial::qwt::qwt _qwt_vcpkg_ok)
if(_qwt_vcpkg_ok)
  set(Qwt_FOUND TRUE)
  set(Qwt_LIBRARIES unofficial::qwt::qwt)
  get_target_property(_qwt_iface_inc unofficial::qwt::qwt INTERFACE_INCLUDE_DIRECTORIES)
  set(Qwt_INCLUDE_DIRS "${_qwt_iface_inc}")
  if(EXISTS "${_qwt_iface_inc}/qwt/qwt_plot.h")
    list(APPEND Qwt_INCLUDE_DIRS "${_qwt_iface_inc}/qwt")
  endif()
  set(Qwt5_INCLUDE_DIR "${_qwt_iface_inc}" CACHE PATH "Qwt include root (legacy name for installers)" FORCE)
  if(NOT Qwt_FIND_QUIETLY)
    message(STATUS "Found Qwt: unofficial-qwt (${Qwt_LIBRARIES})")
  endif()
  return()
endif()

find_package(Qwt CONFIG QUIET)
if(Qwt_FOUND AND TARGET Qwt::Qwt)
  set(Qwt_LIBRARIES Qwt::Qwt)
  get_target_property(_qwt_iface_inc Qwt::Qwt INTERFACE_INCLUDE_DIRECTORIES)
  set(Qwt_INCLUDE_DIRS "${_qwt_iface_inc}")
  if(EXISTS "${_qwt_iface_inc}/qwt/qwt_plot.h")
    list(APPEND Qwt_INCLUDE_DIRS "${_qwt_iface_inc}/qwt")
  endif()
  set(Qwt5_INCLUDE_DIR "${_qwt_iface_inc}" CACHE PATH "Qwt include root (legacy name for installers)" FORCE)
  if(NOT Qwt_FIND_QUIETLY)
    message(STATUS "Found Qwt: ${Qwt_LIBRARIES}")
  endif()
  return()
endif()

find_path(QWT_INCLUDE_DIR
  NAMES qwt/qwt_plot.h
  PATHS ${CMAKE_PREFIX_PATH}
  PATH_SUFFIXES include
  DOC "Qwt headers (directory containing qwt/qwt_plot.h)"
)
find_library(QWT_LIBRARY_RELEASE
  NAMES qwt
  PATHS ${CMAKE_PREFIX_PATH}
  PATH_SUFFIXES lib
)
find_library(QWT_LIBRARY_DEBUG
  NAMES qwtd qwt
  PATHS ${CMAKE_PREFIX_PATH}
  PATH_SUFFIXES debug/lib lib
)

if(QWT_INCLUDE_DIR AND QWT_LIBRARY_RELEASE)
  set(Qwt_FOUND TRUE)
  set(Qwt_INCLUDE_DIRS "${QWT_INCLUDE_DIR}")
  if(EXISTS "${QWT_INCLUDE_DIR}/qwt")
    list(APPEND Qwt_INCLUDE_DIRS "${QWT_INCLUDE_DIR}/qwt")
  endif()
  set(Qwt5_INCLUDE_DIR "${QWT_INCLUDE_DIR}" CACHE PATH "Qwt include root (legacy name for installers)" FORCE)
  if(NOT TARGET CrimsonNetlistEditorQwt)
    add_library(CrimsonNetlistEditorQwt UNKNOWN IMPORTED)
    set_target_properties(CrimsonNetlistEditorQwt PROPERTIES
      IMPORTED_LOCATION "${QWT_LIBRARY_RELEASE}"
      INTERFACE_INCLUDE_DIRECTORIES "${Qwt_INCLUDE_DIRS}"
    )
    if(QWT_LIBRARY_DEBUG)
      set_target_properties(CrimsonNetlistEditorQwt PROPERTIES
        IMPORTED_LOCATION_DEBUG "${QWT_LIBRARY_DEBUG}"
      )
    elseif(CMAKE_CONFIGURATION_TYPES)
      # MSVC multi-config: fall back so Debug configures; prefer installing qwtd.lib via vcpkg when available.
      set_target_properties(CrimsonNetlistEditorQwt PROPERTIES
        IMPORTED_LOCATION_DEBUG "${QWT_LIBRARY_RELEASE}"
      )
    endif()
  endif()
  set(Qwt_LIBRARIES CrimsonNetlistEditorQwt)
  if(NOT Qwt_FIND_QUIETLY)
    message(STATUS "Found Qwt: ${QWT_LIBRARY_RELEASE} (includes: ${QWT_INCLUDE_DIR})")
  endif()
  return()
endif()

find_package(Qt4 QUIET)
if(QT4_FOUND)
  find_package(Qwt5 ${Qwt_FIND_VERSION} QUIET)
  if(Qwt5_FOUND)
    set(Qwt_FOUND TRUE)
    set(Qwt_INCLUDE_DIRS "${Qwt5_INCLUDE_DIR}")
    set(Qwt_LIBRARIES "${Qwt5_Qt4_LIBRARY}")
    set(Qwt5_INCLUDE_DIR "${Qwt5_INCLUDE_DIR}" CACHE PATH "Qwt include root (legacy name for installers)" FORCE)
    if(NOT Qwt_FIND_QUIETLY)
      message(STATUS "Found Qwt: ${Qwt_LIBRARIES}")
    endif()
    return()
  endif()
endif()

if(NOT Qwt_FOUND AND Qwt_FIND_REQUIRED)
  message(FATAL_ERROR "Qwt not found. With Qt6 on Windows, install vcpkg package \"qwt\" and add installed/<triplet> to CMAKE_PREFIX_PATH.")
endif()
