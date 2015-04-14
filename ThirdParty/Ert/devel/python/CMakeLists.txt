include(cmake/find_python_module.cmake)

if (ERT_BUILD_GUI)
   FIND_PACKAGE(PythonInterp 2.7 REQUIRED)
   find_python_module(PyQt4 REQUIRED)
else()
   FIND_PACKAGE(PythonInterp 2.7 REQUIRED)     
endif()

set(PYTHON_INSTALL_PREFIX  "lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages"  CACHE STRING "Subdirectory to install Python modules in")

if (BUILD_TESTS)
   add_subdirectory( test )
endif()
add_subdirectory( python )
