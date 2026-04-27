MITK_CREATE_MODULE_TESTS()

foreach(_crimson_skd IN ITEMS MitkSolidKernelTestDriver SolidKernelTestDriver)
  if(TARGET ${_crimson_skd})
    set_target_properties(${_crimson_skd} PROPERTIES WIN32_EXECUTABLE OFF)
  endif()
endforeach()

configure_file("OCCTestingConfig.h.in" "OCCTestingConfig.h")