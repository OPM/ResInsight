include(cmake/find_python_module.cmake)
include(cmake/python_package_versions.cmake) # finds version

FIND_PACKAGE(PythonInterp 2.7 EXACT)
if (NOT DEFINED PYTHON_EXECUTABLE)
   message("Python2.7 interpreter not found - Python wrappers not enabled")
   return()
endif()


python_package(numpy 1.7.1)
if (NOT DEFINED PY_numpy)
   message("numpy module not found - Python wrappers not enabled")
   return()
endif()

if (EXISTS "/etc/debian_version")
   set( PYTHON_PACKAGE_PATH "dist-packages")
else()
   set( PYTHON_PACKAGE_PATH "site-packages")
endif()
set(PYTHON_INSTALL_PREFIX  "lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/${PYTHON_PACKAGE_PATH}"  CACHE STRING "Subdirectory to install Python modules in")


if (BUILD_TESTS)
   add_subdirectory( tests )
endif()
add_subdirectory( python )
