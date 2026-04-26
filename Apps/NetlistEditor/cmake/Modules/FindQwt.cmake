# FindQwt: bridge Qwt to the legacy variables used by Apps/NetlistEditor/src/CMakeLists.txt
# (Qwt_INCLUDE_DIRS, Qwt_LIBRARIES, Qwt5_INCLUDE_DIR for packaging paths).
#
# - vcpkg installs Qwt 6 as CMake package "unofficial-qwt" (target unofficial::qwt::qwt).
# - Some layouts ship QwtConfig.cmake (CONFIG mode).
# - Legacy Qwt 5 + Qt4 remains available via FindQwt5.cmake when Qt4 is present.
#
# External: for MSVC + Qt6, typical setup is: vcpkg install qwt, then ensure the installed
# triplet root is on CMAKE_PREFIX_PATH (Superbuild already prepends vcpkg paths in some configs).

# vcpkg / Qt6 (preferred on Windows with this project)
find_package(unofficial-qwt CONFIG QUIET)
if(TARGET unofficial::qwt::qwt)
  set(Qwt_FOUND TRUE)
  set(Qwt_LIBRARIES unofficial::qwt::qwt)
  get_target_property(_qwt_iface_inc unofficial::qwt::qwt INTERFACE_INCLUDE_DIRECTORIES)
  set(Qwt_INCLUDE_DIRS "${_qwt_iface_inc}")
  # Qwt 6 headers are usually under include/qwt; sources include <qwt_plot.h>.
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
  message(FATAL_ERROR "Qwt not found. With Qt6 on Windows, install vcpkg package \"qwt\" (provides unofficial-qwt) and add the vcpkg installed directory to CMAKE_PREFIX_PATH.")
endif()
