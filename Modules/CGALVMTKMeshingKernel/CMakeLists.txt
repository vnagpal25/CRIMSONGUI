message("Start of Modules/CGALVMTKMeshingKernel/CMakeLists.txt")

MITK_CREATE_MODULE( CGALVMTKMeshingKernel 
  INCLUDE_DIRS DataManagement ExtensionPoint IO Rendering
  DEPENDS MitkSceneSerializationBase MitkMapperExt SolidKernel
  PACKAGE_DEPENDS PUBLIC GSL OCC Boost VMTK VTK|FiltersVerdict CGAL Qt6|Widgets+Core
  PRIVATE Eigen3 WM5
)

# MSVC: IMeshingKernel.cpp and tetgen.cxx can exceed the default .obj section limit in any config
# when using multi-config generators (CMAKE_BUILD_TYPE may be empty at configure time).
if (MSVC)
  target_compile_options(${MODULE_TARGET} PRIVATE /bigobj)
endif()

# CGAL can pull ITK-bundled Eigen before our .cpp defines run; set globally for this target (MSVC /WX).
# Also force-empty EIGEN_NOEXCEPT for legacy ITK-Eigen + MSVC parser compatibility.
target_compile_definitions(${MODULE_TARGET} PRIVATE TETLIBRARY EIGEN_DONT_USE_CXX11_NOEXCEPT EIGEN_NOEXCEPT=)

message("End of Modules/CGALVMTKMeshingKernel/CMakeLists.txt")