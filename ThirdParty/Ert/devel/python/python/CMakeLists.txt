include(cmake/find_python_module.cmake)
if (ERT_BUILD_GUI)
   FIND_PACKAGE(PythonInterp 2.6 REQUIRED)
   find_python_module(PyQt4 REQUIRED)
else()
   FIND_PACKAGE(PythonInterp 2.6 REQUIRED)     
endif()

add_subdirectory( ert )
if (ERT_BUILD_GUI)
   add_subdirectory( ert_gui )
endif()

