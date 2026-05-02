# - Try to find OCC libraries 
### Does not test what version has been found,though 
### that could be done by parsing Standard_Version.hxx 
 
 
# Once done, this will define 
#  OCC_FOUND - true if OCC has been found 
#  OCC_INCLUDE_DIR - the OCC include dir 
#  OCC_LIBRARIES - names of OCC libraries 
#  OCC_LINK_DIRECTORY - location of OCC libraries 

if (NOT OCC_FOUND AND NOT OCC_DIR)
    SET(OCC_DIR "" CACHE PATH "OpenCascade installation directory" FORCE)
    if (OCC_FIND_REQUIRED)
        message(FATAL_ERROR "Please set OCC_DIR variable to the root of the OpenCascade installation directory")
    endif()
else()
    ########################################################
    ### BEGIN FROM OpenCASCADE's CMakeLists.txt

    if (MSVC)
      if (MSVC70)
        set (COMPILER vc7)
      elseif (MSVC80)
        set (COMPILER vc8)
      elseif (MSVC90)
        set (COMPILER vc9)
      elseif (MSVC10)
        set (COMPILER vc10)
      elseif (MSVC11)
        set (COMPILER vc11)
      elseif (MSVC12)
        set (COMPILER vc12)
      endif()
    elseif (DEFINED CMAKE_COMPILER_IS_GNUCC)
      set (COMPILER gcc)
    elseif (DEFINED CMAKE_COMPILER_IS_GNUCXX)
      set (COMPILER gxx)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      set (COMPILER clang)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
      set (COMPILER icc)
    else()
      set (COMPILER ${CMAKE_GENERATOR})
      string (REGEX REPLACE " " "" COMPILER ${COMPILER})
    endif()


    MATH(EXPR COMPILER_BITNESS "32 + 32*(${CMAKE_SIZEOF_VOID_P}/8)")
    if (WIN32)
      SET(OS_WITH_BIT "win${COMPILER_BITNESS}")
    elseif(APPLE)
      SET(OS_WITH_BIT "mac${COMPILER_BITNESS}")
    else()
      SET(OS_WITH_BIT "lin${COMPILER_BITNESS}")
    endif()

    # MSVC14+ (VS2015 and later) is not covered by the MSVC70..MSVC12 mapping above, which
    # leaves COMPILER empty and breaks paths like ${OCC_DIR}/win64/${COMPILER}/libd.
    # The superbuilt OCCT layout under OCC_DIR uses a vc* directory (often vc14); detect it.
    if(MSVC AND OCC_DIR)
      if(NOT COMPILER)
        foreach(_occ_vc vc143 vc142 vc141 vc14 vc12 vc11 vc10)
          if(EXISTS "${OCC_DIR}/${OS_WITH_BIT}/${_occ_vc}/libd/TKernel.lib")
            set(COMPILER ${_occ_vc})
            break()
          endif()
          if(EXISTS "${OCC_DIR}/${OS_WITH_BIT}/${_occ_vc}/lib/TKernel.lib")
            set(COMPILER ${_occ_vc})
            break()
          endif()
        endforeach()
      endif()
      # Install layouts vary (toolset folder name); accept any win64/* dir that contains TKernel.lib.
      if(NOT COMPILER AND EXISTS "${OCC_DIR}/${OS_WITH_BIT}")
        file(GLOB _occ_toolset_dirs RELATIVE "${OCC_DIR}/${OS_WITH_BIT}" "${OCC_DIR}/${OS_WITH_BIT}/*")
        foreach(_t ${_occ_toolset_dirs})
          if(IS_DIRECTORY "${OCC_DIR}/${OS_WITH_BIT}/${_t}" AND
              (EXISTS "${OCC_DIR}/${OS_WITH_BIT}/${_t}/lib/TKernel.lib" OR
               EXISTS "${OCC_DIR}/${OS_WITH_BIT}/${_t}/libd/TKernel.lib" OR
               EXISTS "${OCC_DIR}/${OS_WITH_BIT}/${_t}/libd/Debug/TKernel.lib"))
            set(COMPILER "${_t}")
            break()
          endif()
        endforeach()
      endif()
    endif()
    
    ### END FROM OpenCASCADE's CMakeLists.txt
    ########################################################

    SET(BUILD_SUFFIX_debug "d")
    SET(BUILD_SUFFIX_release "")

    # find the include dir by looking for Standard_Real.hxx (OCCT install vs some package layouts)
    FIND_PATH( OCC_INCLUDE_DIR Standard_Real.hxx
      PATHS ${OCC_DIR}/inc ${OCC_DIR}/include ${OCC_DIR}/include/opencascade
      DOC "Path to OCC includes" NO_DEFAULT_PATH)

    FIND_PATH( OCC_LINK_DIRECTORY_debug NAMES TKernel.lib libTKernel.so
      PATHS
        "${OCC_DIR}/${OS_WITH_BIT}/${COMPILER}/lib${BUILD_SUFFIX_debug}/Debug"
        "${OCC_DIR}/${OS_WITH_BIT}/${COMPILER}/lib${BUILD_SUFFIX_debug}"
      DOC "Path to OCC debug libs" NO_DEFAULT_PATH)
    FIND_PATH( OCC_LINK_DIRECTORY_release NAMES TKernel.lib libTKernel.so
      PATHS
        "${OCC_DIR}/${OS_WITH_BIT}/${COMPILER}/lib${BUILD_SUFFIX_release}/Release"
        "${OCC_DIR}/${OS_WITH_BIT}/${COMPILER}/lib${BUILD_SUFFIX_release}"
      DOC "Path to OCC release libs" NO_DEFAULT_PATH)
    FIND_PATH( OCC_BINARY_DIRECTORY_debug NAMES TKernel.dll libTKernel.so
      PATHS
        "${OCC_DIR}/${OS_WITH_BIT}/${COMPILER}/bin${BUILD_SUFFIX_debug}/Debug"
        "${OCC_DIR}/${OS_WITH_BIT}/${COMPILER}/bin${BUILD_SUFFIX_debug}"
      DOC "Path to OCC debug DLLs" NO_DEFAULT_PATH)
    FIND_PATH( OCC_BINARY_DIRECTORY_release NAMES TKernel.dll libTKernel.so
      PATHS
        "${OCC_DIR}/${OS_WITH_BIT}/${COMPILER}/bin${BUILD_SUFFIX_release}/Release"
        "${OCC_DIR}/${OS_WITH_BIT}/${COMPILER}/bin${BUILD_SUFFIX_release}"
      DOC "Path to OCC release DLLs" NO_DEFAULT_PATH)
    if(OCC_LINK_DIRECTORY_release AND NOT OCC_LINK_DIRECTORY_relwithdebinfo)
      set(OCC_LINK_DIRECTORY_relwithdebinfo "${OCC_LINK_DIRECTORY_release}" CACHE PATH "Path to OCC RelWithDebInfo libs" FORCE)
    endif()
    if(OCC_BINARY_DIRECTORY_release AND NOT OCC_BINARY_DIRECTORY_relwithdebinfo)
      set(OCC_BINARY_DIRECTORY_relwithdebinfo "${OCC_BINARY_DIRECTORY_release}" CACHE PATH "Path to OCC RelWithDebInfo DLLs" FORCE)
    endif()
    MARK_AS_ADVANCED(OCC_INCLUDE_DIR)
    MARK_AS_ADVANCED(OCC_LINK_DIRECTORY_debug)
    MARK_AS_ADVANCED(OCC_LINK_DIRECTORY_release)
    MARK_AS_ADVANCED(OCC_LINK_DIRECTORY_relwithdebinfo)
    MARK_AS_ADVANCED(OCC_BINARY_DIRECTORY_debug)
    MARK_AS_ADVANCED(OCC_BINARY_DIRECTORY_release)
    MARK_AS_ADVANCED(OCC_BINARY_DIRECTORY_relwithdebinfo)

    IF ( OCC_INCLUDE_DIR AND (OCC_LINK_DIRECTORY_debug OR OCC_LINK_DIRECTORY_release) ) 

        IF( NOT OCC_FIND_COMPONENTS ) 
            set(OCC_FIND_COMPONENTS TKFillet TKMesh TKernel TKG2d TKG3d TKMath TKIGES TKSTL TKShHealing TKXSBase TKBool TKBO TKBRep TKTopAlgo TKGeomAlgo TKGeomBase TKOffset TKPrim TKSTEP TKSTEPBase TKSTEPAttr TKHLR TKFeat TKNIS TKCAF TKLCAF)
        ENDIF()                                                                                                                                                                                                                     
        
        FOREACH( _libname ${OCC_FIND_COMPONENTS} ) 
            foreach (conf debug release)
                IF (NOT OCC_${_libname}_${conf}_FOUND)
                    FIND_LIBRARY( ${_libname}_${conf}_OCCLIB ${_libname} PATHS ${OCC_LINK_DIRECTORY_${conf}} NO_DEFAULT_PATH)
                    SET( _foundlib_${conf} ${${_libname}_${conf}_OCCLIB} ) 
                    if (_foundlib_${conf})
                        set(OCC_${_libname}_${conf}_FOUND TRUE)
                        set(OCC_${_libname}_FOUND TRUE)
                    endif()
                    MARK_AS_ADVANCED(${_libname}_${conf}_OCCLIB)
                ENDIF()
            endforeach ()
            if (OCC_${_libname}_debug_FOUND AND OCC_${_libname}_release_FOUND)
                SET(OCC_LIBRARIES ${OCC_LIBRARIES} debug ${${_libname}_debug_OCCLIB} optimized ${${_libname}_release_OCCLIB}) 
            elseif (OCC_${_libname}_debug_FOUND)
                SET(OCC_LIBRARIES ${OCC_LIBRARIES} ${${_libname}_debug_OCCLIB}) 
            elseif (OCC_${_libname}_release_FOUND)
                SET(OCC_LIBRARIES ${OCC_LIBRARIES} ${${_libname}_release_OCCLIB}) 
            endif()
        ENDFOREACH( _libname ${OCC_FIND_COMPONENTS} ) 

        IF (UNIX) 
            ADD_DEFINITIONS( -DLIN -DLININTEL ) 
        ELSEIF (WIN32) 
            ADD_DEFINITIONS( -DWNT ) 
        ENDIF (UNIX) 

        # 32 bit or 64 bit? 
        IF( CMAKE_SIZEOF_VOID_P EQUAL 8 ) 
            ADD_DEFINITIONS( -D_OCC64 ) 
            IF (UNIX) 
                ADD_DEFINITIONS( -m64 ) 
            ENDIF (UNIX) 
        ENDIF( ) 

        ADD_DEFINITIONS( -DHAVE_CONFIG_H -DHAVE_IOSTREAM -DHAVE_FSTREAM -DHAVE_LIMITS_H ) 
    ENDIF( ) 
endif()    

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OCC REQUIRED_VARS OCC_INCLUDE_DIR OCC_LIBRARIES HANDLE_COMPONENTS)

MARK_AS_ADVANCED(OCC_LIBRARIES)
