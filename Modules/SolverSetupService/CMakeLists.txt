MITK_CREATE_MODULE( SolverSetupService
  DEPENDS MitkCore CRIMSONUtils SolidKernel
  PACKAGE_DEPENDS PUBLIC Qt6|Core GSL
)

#IF( BUILD_TESTING )
#  add_subdirectory(Testing)
#ENDIF()
