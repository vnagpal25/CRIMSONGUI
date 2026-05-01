MITK_CREATE_MODULE( PCMRIKernel
  INCLUDE_DIRS DataManagement ExtensionPoint ExtensionPoint/internal Rendering IO # Initialization Interactions
  DEPENDS MitkSceneSerializationBase MitkPlanarFigure MitkSegmentation AsyncTask MitkContourModel SolidKernel ${CRIMSON_MESHING_KERNEL}MeshingKernel
  PACKAGE_DEPENDS PUBLIC GSL Boost Eigen3
  PRIVATE WM5 Qt6 Qt6|Core #Cholmod
)

if (MSVC)
  target_compile_options(${MODULE_TARGET} PRIVATE /bigobj)
endif()

IF( BUILD_TESTING )
#  add_subdirectory(Testing)
ENDIF()
