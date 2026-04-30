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

# TETLIBRARY: tetgen.cxx / wrapper. (ITK 5.4 itkeigen no longer honors EIGEN_DONT_USE_CXX11_NOEXCEPT; MSVC C3646 on
# EIGEN_NOEXCEPT is fixed by including Eigen/Core before CGAL/MITK in IMeshingKernel.cpp.)
target_compile_definitions(${MODULE_TARGET} PRIVATE TETLIBRARY)

message("End of Modules/CGALVMTKMeshingKernel/CMakeLists.txt")