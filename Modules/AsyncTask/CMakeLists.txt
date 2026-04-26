MITK_CREATE_MODULE( AsyncTask 
    PACKAGE_DEPENDS PUBLIC GSL Boost )

# Header-only Boost (signals2) needs include dirs when the module graph does not pull Boost::headers.
set(_crimson_async_task_tgt "")
if(TARGET MitkAsyncTask)
  set(_crimson_async_task_tgt MitkAsyncTask)
elseif(TARGET AsyncTask)
  set(_crimson_async_task_tgt AsyncTask)
endif()
if(_crimson_async_task_tgt)
  if(TARGET Boost::headers)
    target_link_libraries(${_crimson_async_task_tgt} PUBLIC Boost::headers)
  elseif(TARGET Boost::boost)
    target_link_libraries(${_crimson_async_task_tgt} PUBLIC Boost::boost)
  elseif(Boost_INCLUDE_DIRS)
    # Pre-built MITK / FindBoost often exposes only variables; PACKAGE_DEPENDS may not add includes to this target.
    target_include_directories(${_crimson_async_task_tgt} SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
  elseif(Boost_INCLUDE_DIR)
    target_include_directories(${_crimson_async_task_tgt} SYSTEM PUBLIC ${Boost_INCLUDE_DIR})
  endif()
endif()

IF( BUILD_TESTING )
#add_subdirectory(Testing)
ENDIF()
