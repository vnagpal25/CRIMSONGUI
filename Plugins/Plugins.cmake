# PythonSolverSetupService is only created when MITK was built with Python (MitkPython, etc.).
# If MITK lacks Python, skip these plugins so CRIMSON-Configure can complete; enable MITK_USE_Python and Python dev libs to build them.
set(_crimson_solver_plugins ON)
if(NOT TARGET PythonSolverSetupService)
  set(_crimson_solver_plugins OFF)
  message(WARNING "PythonSolverSetupService is not available (build MITK with MITK_USE_Python=ON and Python development files). SolverSetup plugins are disabled.")
endif()

set(PROJECT_PLUGINS
  Plugins/uk.ac.kcl.CRIMSONApp:ON
  Plugins/uk.ac.kcl.AsyncTaskManager:ON
  Plugins/uk.ac.kcl.AsyncTaskManagerView:ON
  Plugins/uk.ac.kcl.HierarchyManager:ON
  Plugins/uk.ac.kcl.VascularModeling.Eager:ON
  Plugins/uk.ac.kcl.VascularModeling:ON
  Plugins/uk.ac.kcl.VesselMeshing.Eager:ON
  Plugins/uk.ac.kcl.VesselMeshing:ON
  Plugins/uk.ac.kcl.SolverSetup:${_crimson_solver_plugins}
  Plugins/uk.ac.kcl.SolverSetupView:${_crimson_solver_plugins}
  Plugins/uk.ac.kcl.SolverSetupPython:${_crimson_solver_plugins}
)
