MITK_CREATE_MODULE_TESTS()

# MITK's test driver must be a console app on Windows (otherwise the linker expects WinMain).
foreach(_crimson_vtd IN ITEMS MitkVesselTreeTestDriver VesselTreeTestDriver)
  if(TARGET ${_crimson_vtd})
    set_target_properties(${_crimson_vtd} PROPERTIES WIN32_EXECUTABLE OFF)
  endif()
endforeach()