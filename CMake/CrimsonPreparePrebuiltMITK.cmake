#-----------------------------------------------------------------------------
# Prepare Boost and prefix paths before find_package(MITK) when using a
# pre-built MITK (MITK_DIR points at MITK-build). CGAL is fixed to vcpkg below.
#
# The superbuild includes CMakeExternals/MITK.cmake, but CRIMSON-Configure
# re-runs the top-level CMakeLists.txt with CRIMSON_USE_SUPERBUILD=OFF and
# never loads MITK.cmake — so the same preparation must run there as well.
#
# When MITK's ../ep tree has no BoostConfig.cmake (typical b2 layout), we set
# CMP0167 OLD and BOOST_ROOT so the FindBoost module can run. CMake 3.30+
# removed the in-tree FindBoost module; CRIMSON ships CMake/FindBoost.cmake
# (from CMake 3.29.6) on CMAKE_MODULE_PATH for that case.
#-----------------------------------------------------------------------------

macro(crimson_prepare_prebuilt_mitk)
  set(_crimson_vcpkg "C:/vcpkg/installed/x64-windows")
  set(CGAL_DIR "${_crimson_vcpkg}/share/cgal" CACHE PATH "Directory containing CGALConfig.cmake (vcpkg)" FORCE)
  list(PREPEND CMAKE_PREFIX_PATH "${_crimson_vcpkg}")
  if(MITK_DIR)
    get_filename_component(_crimson_mitk_tree "${MITK_DIR}/.." ABSOLUTE)
    if(EXISTS "${_crimson_mitk_tree}/ep")
      list(PREPEND CMAKE_PREFIX_PATH "${_crimson_mitk_tree}/ep")
    endif()
    if(EXISTS "${MITK_DIR}/ep")
      list(PREPEND CMAKE_PREFIX_PATH "${MITK_DIR}/ep")
    endif()

    if(NOT DEFINED MITK_BOOST_ROOT)
      set(MITK_BOOST_ROOT "" CACHE PATH "Boost install prefix when using pre-built MITK (optional if Boost is under MITK ../ep)")
    endif()
    if(MITK_BOOST_ROOT)
      set(BOOST_ROOT "${MITK_BOOST_ROOT}" CACHE PATH "" FORCE)
      set(Boost_ROOT "${MITK_BOOST_ROOT}" CACHE PATH "" FORCE)
      list(PREPEND CMAKE_PREFIX_PATH "${MITK_BOOST_ROOT}")
    elseif(BOOST_ROOT)
      list(PREPEND CMAKE_PREFIX_PATH "${BOOST_ROOT}")
    endif()

    set(_crimson_boost_configs)
    if(EXISTS "${_crimson_mitk_tree}/ep")
      file(GLOB _crimson_b1 "${_crimson_mitk_tree}/ep/lib/cmake/Boost-*/BoostConfig.cmake")
      list(APPEND _crimson_boost_configs ${_crimson_b1})
    endif()
    foreach(_crimson_br IN ITEMS "${MITK_BOOST_ROOT}" "${BOOST_ROOT}")
      if(_crimson_br AND EXISTS "${_crimson_br}")
        file(GLOB _crimson_bx "${_crimson_br}/lib/cmake/Boost-*/BoostConfig.cmake")
        list(APPEND _crimson_boost_configs ${_crimson_bx})
      endif()
    endforeach()
    if(_crimson_boost_configs)
      list(REMOVE_DUPLICATES _crimson_boost_configs)
      list(SORT _crimson_boost_configs)
      list(GET _crimson_boost_configs -1 _crimson_boost_config_file)
      get_filename_component(_crimson_boost_dir "${_crimson_boost_config_file}" DIRECTORY)
      set(Boost_DIR "${_crimson_boost_dir}" CACHE PATH "Boost CMake package (pre-built MITK)" FORCE)
    endif()

    cmake_policy(SET CMP0074 NEW)
    if(POLICY CMP0167)
      if(Boost_DIR AND EXISTS "${Boost_DIR}/BoostConfig.cmake")
        cmake_policy(SET CMP0167 NEW)
      else()
        cmake_policy(SET CMP0167 OLD)
        if(NOT BOOST_ROOT AND EXISTS "${_crimson_mitk_tree}/ep/include/boost/version.hpp")
          set(BOOST_ROOT "${_crimson_mitk_tree}/ep" CACHE PATH "" FORCE)
        endif()
      endif()
    endif()
  endif()
endmacro()
