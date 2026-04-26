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
  endif()
endif()

IF( BUILD_TESTING )
#add_subdirectory(Testing)
ENDIF()
