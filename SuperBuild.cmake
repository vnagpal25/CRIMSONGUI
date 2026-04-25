#-----------------------------------------------------------------------------
# Convenient macro allowing to download a file
#-----------------------------------------------------------------------------

if(NOT MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL)
  set(MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL http://mitk.org/download/thirdparty)
endif()

macro(downloadFile url dest)
  file(DOWNLOAD ${url} ${dest} STATUS status)
  list(GET status 0 error_code)
  list(GET status 1 error_msg)
  if(error_code)
    message(FATAL_ERROR "error: Failed to download ${url} - ${error_msg}")
  endif()
endmacro()

# We need GNU patch for ExternalProject PATCH_COMMAND (OCC, freetype, etc.).
# Pre-built MITK does not skip this: the CRIMSON superbuild still patches other deps.
# On Windows, prefer Git for Windows' patch.exe (usually not on PATH in VS prompts).
set(CRIMSON_PATCH_EXECUTABLE "" CACHE FILEPATH
  "Optional: full path to GNU patch (patch.exe). Set this if configure cannot find patch and mitk.org download fails.")

set(PATCH_COMMAND "")
if(CRIMSON_PATCH_EXECUTABLE AND EXISTS "${CRIMSON_PATCH_EXECUTABLE}")
  set(PATCH_COMMAND "${CRIMSON_PATCH_EXECUTABLE}")
endif()

if(NOT PATCH_COMMAND)
  if(WIN32)
    find_program(PATCH_COMMAND NAMES patch patch.exe
      HINTS
        "$ENV{ProgramFiles}/Git/usr/bin"
        "$ENV{ProgramFiles}/Git/mingw64/bin"
        "$ENV{LOCALAPPDATA}/Programs/Git/usr/bin"
        "$ENV{SystemDrive}/Program Files/Git/usr/bin"
        "$ENV{SystemDrive}/Program Files (x86)/Git/usr/bin"
    )
  endif()
endif()
if(NOT PATCH_COMMAND)
  find_program(PATCH_COMMAND NAMES patch patch.exe)
endif()

if((NOT PATCH_COMMAND OR NOT EXISTS "${PATCH_COMMAND}") AND WIN32)
  set(_crimson_patch_urls
    "${MITK_THIRDPARTY_DOWNLOAD_PREFIX_URL}/patch.exe"
    "https://www.mitk.org/download/thirdparty/patch.exe"
    "http://www.mitk.org/download/thirdparty/patch.exe"
  )
  set(_crimson_patch_dest "${CMAKE_CURRENT_BINARY_DIR}/patch.exe")
  set(_crimson_patch_ok FALSE)
  foreach(_u IN LISTS _crimson_patch_urls)
    file(DOWNLOAD "${_u}" "${_crimson_patch_dest}" STATUS _st TLS_VERIFY ON)
    list(GET _st 0 _code)
    if(NOT _code)
      set(_crimson_patch_ok TRUE)
      break()
    endif()
  endforeach()
  if(_crimson_patch_ok)
    find_program(PATCH_COMMAND NAMES patch patch.exe HINTS "${CMAKE_CURRENT_BINARY_DIR}" NO_DEFAULT_PATH)
  else()
    message(FATAL_ERROR
      "No patch program found and download of patch.exe failed (mitk.org may be unreachable).\n"
      "  Fix (pick one):\n"
      "  - Install Git for Windows (includes usr/bin/patch.exe), then re-run CMake, or\n"
      "  - Set CRIMSON_PATCH_EXECUTABLE to patch.exe from Git (e.g. .../Git/usr/bin/patch.exe), or\n"
      "  - Add the directory containing GNU patch to PATH.\n"
      "MITK_DIR / EXTERNAL_MITK_DIR only skips building MITK; OCC and other externals still need patch.")
  endif()
endif()
if(NOT PATCH_COMMAND OR NOT EXISTS "${PATCH_COMMAND}")
  message(FATAL_ERROR "No patch program found.")
endif()

#-----------------------------------------------------------------------------
# ExternalProjects
#-----------------------------------------------------------------------------

set(PACKAGE_FLOWSOLVER OFF CACHE BOOL "Download the flowsolver as part of the build process (not currently supported)")

set(external_projects
  freetype
  TBB
  OCC
  WM5
  QtPropertyBrowser
  presolver
  GSL
  )

if(PACKAGE_FLOWSOLVER)
  list(APPEND external_projects flowsolver)
endif()

if(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  list(APPEND external_projects PythonModules parse)
endif()
  

set(EXTERNAL_MITK_DIR "C:/v/MITK-sb/MITK-build" CACHE PATH "MITK-build directory (contains MITKConfig.cmake); leave empty to build MITK here")
mark_as_advanced(EXTERNAL_MITK_DIR)
if(EXTERNAL_MITK_DIR)
  set(MITK_DIR "${EXTERNAL_MITK_DIR}")
endif()

# Look for git early on, if needed
if(NOT MITK_DIR AND MITK_USE_CTK AND NOT CTK_DIR)
  find_package(Git REQUIRED)
endif()

#-----------------------------------------------------------------------------
# External project settings
#-----------------------------------------------------------------------------

include(ExternalProject)

set(ep_base "${CMAKE_BINARY_DIR}/CMakeExternals")
set_property(DIRECTORY PROPERTY EP_BASE ${ep_base})

set(ep_install_dir "${CMAKE_BINARY_DIR}/CMakeExternals/Install")
set(ep_suffix "-cmake")
set(ep_build_shared_libs ON)
set(ep_build_testing OFF)

# Compute -G arg for configuring external projects with the same CMake generator:
if(CMAKE_EXTRA_GENERATOR)
  set(gen "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
else()
  set(gen "${CMAKE_GENERATOR}")
endif()

# Use this value where semi-colons are needed in ep_add args:
set(sep "^^")

##

if(MSVC_VERSION)
  list(APPEND CMAKE_C_FLAGS "/bigobj" "/MP")
  list(APPEND CMAKE_CXX_FLAGS "/bigobj" "/MP")
else()
  # GCC?
  list(APPEND CMAKE_CXX_FLAGS "-std=c++1y")
endif()

# ExternalProject forwards -DCMAKE_*_FLAGS:STRING=... to nested CMake/VS tools. CMake stores
# flags as semicolon-separated lists; semicolons break argv reconstruction on Windows and
# loose tokens like "/bigobj" are mis-parsed as paths ("Ignoring extra path from command line").
foreach(_sb_flag_var
    CMAKE_C_FLAGS
    CMAKE_CXX_FLAGS
    CMAKE_C_FLAGS_DEBUG
    CMAKE_CXX_FLAGS_DEBUG
    CMAKE_C_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_RELEASE
    CMAKE_C_FLAGS_RELWITHDEBINFO
    CMAKE_CXX_FLAGS_RELWITHDEBINFO
    CMAKE_EXE_LINKER_FLAGS
    CMAKE_SHARED_LINKER_FLAGS
    CMAKE_MODULE_LINKER_FLAGS
  )
  string(REPLACE ";" " " ${_sb_flag_var} "${${_sb_flag_var}}")
endforeach()

set(ep_common_args
  -DBUILD_TESTING:BOOL=${ep_build_testing}
  -DCMAKE_INSTALL_PREFIX:PATH=${ep_install_dir}
  -DBUILD_SHARED_LIBS:BOOL=${ep_build_shared_libs}
  -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
  -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
  -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
  -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
  -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
  # debug flags
  -DCMAKE_CXX_FLAGS_DEBUG:STRING=${CMAKE_CXX_FLAGS_DEBUG}
  -DCMAKE_C_FLAGS_DEBUG:STRING=${CMAKE_C_FLAGS_DEBUG}
  # release flags
  -DCMAKE_CXX_FLAGS_RELEASE:STRING=${CMAKE_CXX_FLAGS_RELEASE}
  -DCMAKE_C_FLAGS_RELEASE:STRING=${CMAKE_C_FLAGS_RELEASE}
  # relwithdebinfo
  -DCMAKE_CXX_FLAGS_RELWITHDEBINFO:STRING=${CMAKE_CXX_FLAGS_RELWITHDEBINFO}
  -DCMAKE_C_FLAGS_RELWITHDEBINFO:STRING=${CMAKE_C_FLAGS_RELWITHDEBINFO}
  # link flags
  -DCMAKE_EXE_LINKER_FLAGS:STRING=${CMAKE_EXE_LINKER_FLAGS}
  -DCMAKE_SHARED_LINKER_FLAGS:STRING=${CMAKE_SHARED_LINKER_FLAGS}
  -DCMAKE_MODULE_LINKER_FLAGS:STRING=${CMAKE_MODULE_LINKER_FLAGS}
)

set(Qt6_DIR "C:/Vansh_Files/Qt/6.11.0/msvc2022_64/lib/cmake/Qt6" CACHE PATH "Path to Qt6 CMake config directory")
list(APPEND CMAKE_PREFIX_PATH "C:/Vansh_Files/Qt/6.11.0/msvc2022_64/lib/cmake")

# Include external projects
foreach(p MITK ${external_projects})
  include(CMakeExternals/${p}.cmake)
endforeach()

# MITK superbuild installs VMTK, CGAL, and other packages under <MITK superbuild root>/ep.
# CRIMSON-Configure must search that tree or find_package(VMTK) fails ("Missing package: VMTK").
# If your MITK tree has no ep/lib/cmake/VMTK, build the VMTK target in the MITK *superbuild*
# solution, or build VMTK separately and set VMTK_DIR to the folder that contains VMTKConfig.cmake.
set(VMTK_DIR "C:/v/vmtk-install/lib" CACHE PATH "Directory containing VMTKConfig.cmake (override in CMake GUI if needed)")
# Folder that contains CGALConfig.cmake (often <prefix>/lib/cmake/CGAL). Build CGAL via MITK superbuild,
# or install CGAL and set this, or leave empty to autodetect under MITK_DIR/../ep/lib/cmake/CGAL.
set(CGAL_DIR "" CACHE PATH "Directory containing CGALConfig.cmake (autodetected from MITK ep when present)")
set(CRIMSON_EXTRA_CMAKE_PREFIX_PATH "" CACHE PATH "Optional extra prefix(es) for CRIMSON-Configure CMAKE_PREFIX_PATH if VMTK/CGAL are not under MITK_DIR/../ep")
set(_crimson_cmake_prefix_path "")
if(MITK_DIR)
  get_filename_component(_mitk_superbuild_root "${MITK_DIR}/.." ABSOLUTE)
  if(EXISTS "${_mitk_superbuild_root}/ep")
    list(APPEND _crimson_cmake_prefix_path "${_mitk_superbuild_root}/ep")
    set(_cgal_guess "${_mitk_superbuild_root}/ep/lib/cmake/CGAL")
    if(EXISTS "${_cgal_guess}/CGALConfig.cmake")
      set(_cgal_use_guess FALSE)
      if(NOT CGAL_DIR)
        set(_cgal_use_guess TRUE)
      elseif(NOT EXISTS "${CGAL_DIR}/CGALConfig.cmake")
        set(_cgal_use_guess TRUE)
      endif()
      if(_cgal_use_guess)
        set(CGAL_DIR "${_cgal_guess}" CACHE PATH "Autodetected from MITK superbuild ep" FORCE)
      endif()
    endif()
  endif()
endif()
if(CRIMSON_EXTRA_CMAKE_PREFIX_PATH)
  list(APPEND _crimson_cmake_prefix_path "${CRIMSON_EXTRA_CMAKE_PREFIX_PATH}")
endif()
if(VMTK_DIR AND EXISTS "${VMTK_DIR}")
  get_filename_component(_vmtk_install_root "${VMTK_DIR}/.." ABSOLUTE)
  list(APPEND _crimson_cmake_prefix_path "${_vmtk_install_root}")
endif()
if(CGAL_DIR AND EXISTS "${CGAL_DIR}/CGALConfig.cmake")
  get_filename_component(_cgal_install_root "${CGAL_DIR}/../.." ABSOLUTE)
  list(APPEND _crimson_cmake_prefix_path "${_cgal_install_root}")
endif()
list(APPEND _crimson_cmake_prefix_path "C:/Vansh_Files/Qt/6.11.0/msvc2022_64/lib/cmake")
list(REMOVE_DUPLICATES _crimson_cmake_prefix_path)
# Passing multi-entry CMAKE_PREFIX_PATH on the cmake command line breaks in two ways on Windows:
# (1) VS/MSBuild can split on ';' so only the first prefix is bound to -DCMAKE_PREFIX_PATH.
# (2) If LIST_SEPARATOR is '^^' and we join prefixes with '^^', ExternalProject splits that
#     into multiple argv tokens, so CMake warns: Ignoring extra path from command line.
# Seed CMAKE_PREFIX_PATH via an initial cache file instead (-C), which avoids both issues.
set(_crimson_configure_initial_cache "${CMAKE_BINARY_DIR}/${MY_PROJECT_NAME}-configure-initial-cache.cmake")
list(JOIN _crimson_cmake_prefix_path ";" _crimson_cmake_prefix_path_for_cache)
file(WRITE "${_crimson_configure_initial_cache}"
"# Generated by SuperBuild.cmake — do not edit
set(CMAKE_PREFIX_PATH \"${_crimson_cmake_prefix_path_for_cache}\" CACHE PATH \"\" FORCE)
")
if(VMTK_DIR)
  file(APPEND "${_crimson_configure_initial_cache}"
    "set(VMTK_DIR \"${VMTK_DIR}\" CACHE PATH \"\" FORCE)\n")
endif()
if(CGAL_DIR AND EXISTS "${CGAL_DIR}/CGALConfig.cmake")
  file(APPEND "${_crimson_configure_initial_cache}"
    "set(CGAL_DIR \"${CGAL_DIR}\" CACHE PATH \"\" FORCE)\n")
endif()

set(_crimson_qmake "")
if(Qt6_DIR)
  get_filename_component(_qt6_prefix "${Qt6_DIR}/../../.." ABSOLUTE)
  find_program(_crimson_qmake NAMES qmake qmake.exe HINTS "${_qt6_prefix}/bin" NO_DEFAULT_PATH)
endif()
if(NOT _crimson_qmake)
  find_program(_crimson_qmake NAMES qmake qmake.exe)
endif()
if(_crimson_qmake)
  file(APPEND "${_crimson_configure_initial_cache}"
    "set(QT_QMAKE_EXECUTABLE \"${_crimson_qmake}\" CACHE FILEPATH \"\" FORCE)\n")
endif()

set(CRIMSON_CONFIGURE_EXTRA_ARGS "")
if(VMTK_DIR)
  list(APPEND CRIMSON_CONFIGURE_EXTRA_ARGS "-DVMTK_DIR:PATH=${VMTK_DIR}")
endif()
if(CGAL_DIR AND EXISTS "${CGAL_DIR}/CGALConfig.cmake")
  list(APPEND CRIMSON_CONFIGURE_EXTRA_ARGS "-DCGAL_DIR:PATH=${CGAL_DIR}")
endif()
if(_crimson_qmake)
  list(APPEND CRIMSON_CONFIGURE_EXTRA_ARGS "-DQT_QMAKE_EXECUTABLE:FILEPATH=${_crimson_qmake}")
endif()

#-----------------------------------------------------------------------------
# Set superbuild boolean args
#-----------------------------------------------------------------------------

set(my_cmake_boolean_args
  WITH_COVERAGE
  BUILD_TESTING
  ${MY_PROJECT_NAME}_BUILD_ALL_PLUGINS
  )

#-----------------------------------------------------------------------------
# Create the final variable containing superbuild boolean args
#-----------------------------------------------------------------------------

set(my_superbuild_boolean_args)
foreach(my_cmake_arg ${my_cmake_boolean_args})
  list(APPEND my_superbuild_boolean_args -D${my_cmake_arg}:BOOL=${${my_cmake_arg}})
endforeach()

#-----------------------------------------------------------------------------
# Project Utilities
#-----------------------------------------------------------------------------

set(proj ${MY_PROJECT_NAME}-Utilities)
ExternalProject_Add(${proj}
  DOWNLOAD_COMMAND ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  DEPENDS
    # Mandatory dependencies
    ${MITK_DEPENDS}
    # Optional dependencies
)

#-----------------------------------------------------------------------------
# Additional Project CXX/C Flags
#-----------------------------------------------------------------------------

set(${MY_PROJECT_NAME}_ADDITIONAL_C_FLAGS "" CACHE STRING "Additional C Flags for ${MY_PROJECT_NAME}")
set(${MY_PROJECT_NAME}_ADDITIONAL_C_FLAGS_RELEASE "" CACHE STRING "Additional Release C Flags for ${MY_PROJECT_NAME}")
set(${MY_PROJECT_NAME}_ADDITIONAL_C_FLAGS_DEBUG "" CACHE STRING "Additional Debug C Flags for ${MY_PROJECT_NAME}")
mark_as_advanced(${MY_PROJECT_NAME}_ADDITIONAL_C_FLAGS ${MY_PROJECT_NAME}_ADDITIONAL_C_FLAGS_DEBUG ${MY_PROJECT_NAME}_ADDITIONAL_C_FLAGS_RELEASE)

set(${MY_PROJECT_NAME}_ADDITIONAL_CXX_FLAGS "" CACHE STRING "Additional CXX Flags for ${MY_PROJECT_NAME}")
set(${MY_PROJECT_NAME}_ADDITIONAL_CXX_FLAGS_RELEASE "" CACHE STRING "Additional Release CXX Flags for ${MY_PROJECT_NAME}")
set(${MY_PROJECT_NAME}_ADDITIONAL_CXX_FLAGS_DEBUG "" CACHE STRING "Additional Debug CXX Flags for ${MY_PROJECT_NAME}")
mark_as_advanced(${MY_PROJECT_NAME}_ADDITIONAL_CXX_FLAGS ${MY_PROJECT_NAME}_ADDITIONAL_CXX_FLAGS_DEBUG ${MY_PROJECT_NAME}_ADDITIONAL_CXX_FLAGS_RELEASE)

set(${MY_PROJECT_NAME}_ADDITIONAL_EXE_LINKER_FLAGS "" CACHE STRING "Additional exe linker flags for ${MY_PROJECT_NAME}")
set(${MY_PROJECT_NAME}_ADDITIONAL_SHARED_LINKER_FLAGS "" CACHE STRING "Additional shared linker flags for ${MY_PROJECT_NAME}")
set(${MY_PROJECT_NAME}_ADDITIONAL_MODULE_LINKER_FLAGS "" CACHE STRING "Additional module linker flags for ${MY_PROJECT_NAME}")
mark_as_advanced(${MY_PROJECT_NAME}_ADDITIONAL_EXE_LINKER_FLAGS ${MY_PROJECT_NAME}_ADDITIONAL_SHARED_LINKER_FLAGS ${MY_PROJECT_NAME}_ADDITIONAL_MODULE_LINKER_FLAGS)

#-----------------------------------------------------------------------------
# CRIMSON-Configure
#-----------------------------------------------------------------------------
# This section is responsible for creating the C:\cb\CRIMSON-build directory,
# and its contents, including CRIMSON.sln in that directory.

set(proj ${MY_PROJECT_NAME}-Configure)

# NOTE: This project "Recycles" ./CMakeLists.txt...
ExternalProject_Add(${proj}
  DOWNLOAD_COMMAND ""
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    -C
    "${_crimson_configure_initial_cache}"
    # --------------- Build options ----------------
    -DBUILD_TESTING:BOOL=${ep_build_testing}
    -DCMAKE_INSTALL_PREFIX:PATH=${ep_install_dir}
    -DBUILD_SHARED_LIBS:BOOL=${ep_build_shared_libs}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    # --------------- Compile options ----------------
    -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
    "-DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS} ${${MY_PROJECT_NAME}_ADDITIONAL_C_FLAGS}"
    "-DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS} ${${MY_PROJECT_NAME}_ADDITIONAL_CXX_FLAGS}"
    # debug flags
    "-DCMAKE_CXX_FLAGS_DEBUG:STRING=${CMAKE_CXX_FLAGS_DEBUG} ${${MY_PROJECT_NAME}_ADDITIONAL_CXX_FLAGS_DEBUG}"
    "-DCMAKE_C_FLAGS_DEBUG:STRING=${CMAKE_C_FLAGS_DEBUG} ${${MY_PROJECT_NAME}_ADDITIONAL_C_FLAGS_DEBUG}"
    # release flags
    "-DCMAKE_CXX_FLAGS_RELEASE:STRING=${CMAKE_CXX_FLAGS_RELEASE} ${${MY_PROJECT_NAME}_ADDITIONAL_CXX_FLAGS_RELEASE}"
    "-DCMAKE_C_FLAGS_RELEASE:STRING=${CMAKE_C_FLAGS_RELEASE} ${${MY_PROJECT_NAME}_ADDITIONAL_C_FLAGS_RELEASE}"
    # relwithdebinfo
    -DCMAKE_CXX_FLAGS_RELWITHDEBINFO:STRING=${CMAKE_CXX_FLAGS_RELWITHDEBINFO}
    -DCMAKE_C_FLAGS_RELWITHDEBINFO:STRING=${CMAKE_C_FLAGS_RELWITHDEBINFO}
    # link flags
    "-DCMAKE_EXE_LINKER_FLAGS:STRING=${CMAKE_EXE_LINKER_FLAGS} ${${MY_PROJECT_NAME}_ADDITIONAL_EXE_LINKER_FLAGS}"
    "-DCMAKE_SHARED_LINKER_FLAGS:STRING=${CMAKE_SHARED_LINKER_FLAGS} ${${MY_PROJECT_NAME}_ADDITIONAL_SHARED_LINKER_FLAGS}"
    "-DCMAKE_MODULE_LINKER_FLAGS:STRING=${CMAKE_MODULE_LINKER_FLAGS} ${${MY_PROJECT_NAME}_ADDITIONAL_MODULE_LINKER_FLAGS}"
    # ------------- Boolean build options --------------
    ${my_superbuild_boolean_args}

    #[AJM]  Deliberately always OFF (even if it's ON for the main script); if SuperBuild isn't OFF for this project, then it'll just re-run the SuperBuild.cmake 
    #       script instead of building CRIMSON itself
    -D${MY_PROJECT_NAME}_USE_SUPERBUILD:BOOL=OFF
    -D${MY_PROJECT_NAME}_CONFIGURED_VIA_SUPERBUILD:BOOL=ON
    -DCTEST_USE_LAUNCHERS:BOOL=${CTEST_USE_LAUNCHERS}
    # ----------------- Miscellaneous ---------------
    -D${MY_PROJECT_NAME}_SUPERBUILD_BINARY_DIR:PATH=${PROJECT_BINARY_DIR}
    -DQt6_DIR:PATH=${Qt6_DIR}
    -DMITK_DIR:PATH=${MITK_DIR}
    -DITK_DIR:PATH=${ITK_DIR}
    -DVTK_DIR:PATH=${VTK_DIR}
    -DOCC_DIR:PATH=${OCC_DIR}
    -DWM5_ROOT_DIR:PATH=${WM5_DIR}
    -DQtPropertyBrowser_DIR:PATH=${QtPropertyBrowser_DIR}
    -DPRESOLVER_EXECUTABLE:FILEPATH=${presolver_executable}
    -DGSL_INCLUDE_DIR:PATH=${GSL_INCLUDE_DIR}
    -DCRIMSON_MESHING_KERNEL:STRING=${CRIMSON_MESHING_KERNEL}
    ${CRIMSON_CONFIGURE_EXTRA_ARGS}

  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} # Since we're running in ./, this will cause cmake to re-run ./CMakeLists.txt!
  BINARY_DIR ${CMAKE_BINARY_DIR}/${MY_PROJECT_NAME}-build
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  DEPENDS
    ${MY_PROJECT_NAME}-Utilities
    ${external_projects}
    )


#-----------------------------------------------------------------------------
# Project
#-----------------------------------------------------------------------------

if(CMAKE_GENERATOR MATCHES ".*Makefiles.*")
  set(_build_cmd "$(MAKE)")
else()
  set(_build_cmd ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/${MY_PROJECT_NAME}-build --config ${CMAKE_CFG_INTDIR})
endif()

# The variable SUPERBUILD_EXCLUDE_${MY_PROJECT_NAME}BUILD_TARGET should be set when submitting to a dashboard
if(NOT DEFINED SUPERBUILD_EXCLUDE_${MY_PROJECT_NAME}BUILD_TARGET OR NOT SUPERBUILD_EXCLUDE_${MY_PROJECT_NAME}BUILD_TARGET)
  set(_target_all_option "ALL")
else()
  set(_target_all_option "")
endif()

add_custom_target(${MY_PROJECT_NAME}-build ${_target_all_option}
  COMMAND ${_build_cmd}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${MY_PROJECT_NAME}-build
  DEPENDS ${MY_PROJECT_NAME}-Configure
  )

#-----------------------------------------------------------------------------
# Custom target allowing to drive the build of the project itself
#-----------------------------------------------------------------------------

add_custom_target(${MY_PROJECT_NAME}
  COMMAND ${_build_cmd}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${MY_PROJECT_NAME}-build
)

