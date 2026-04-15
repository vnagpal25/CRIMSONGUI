#-----------------------------------------------------------------------------
# OpenCascade
#-----------------------------------------------------------------------------

# Sanity checks
if(DEFINED QtPropertyBrowser_DIR AND NOT EXISTS ${QtPropertyBrowser_DIR})
message(FATAL_ERROR "QtPropertyBrowser_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj QtPropertyBrowser)
set(proj_DEPENDENCIES)
set(QtPropertyBrowser_DEPENDS ${proj})

if(NOT DEFINED QtPropertyBrowser_DIR)
    set(additional_args )

    ExternalProject_Add(${proj}
      LIST_SEPARATOR ${sep}
      GIT_REPOSITORY https://github.com/rkhlebnikov/QtPropertyBrowser.git
      PATCH_COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=<SOURCE_DIR> -P ${CMAKE_CURRENT_LIST_DIR}/QtPropertyBrowser_qt6_patch.cmake
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
         ${ep_common_args}
         ${additional_args}
         "-DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS} ${QtPropertyBrowser_CXX_FLAGS}"
         "-DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS} ${QtPropertyBrowser_C_FLAGS}"

         "-DBUILD_CONFIGURATION:STRING=${CMAKE_BUILD_TYPE}"
         "-DQt6_DIR:PATH=${Qt6_DIR}"
         
      CMAKE_CACHE_ARGS
        ${ep_common_cache_args}
      INSTALL_COMMAND ""
      CMAKE_CACHE_DEFAULT_ARGS
        ${ep_common_cache_default_args}
      DEPENDS ${proj_DEPENDENCIES}
      )

    ExternalProject_Get_Property(${proj} binary_dir)
    set(QtPropertyBrowser_DIR ${binary_dir})
      
    #set(QtPropertyBrowser_DIR ${ep_prefix})
    #mitkFunctionInstallExternalCMakeProject(${proj})

else()
    MacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
endif()
