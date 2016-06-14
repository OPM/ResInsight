find_package (PythonInterp REQUIRED)
find_package (ERTPython REQUIRED)

add_subdirectory( c_inter )

if (EXISTS "/etc/debian_version")
   set( PYTHON_PACKAGE_PATH "dist-packages")
else()
   set( PYTHON_PACKAGE_PATH "site-packages")
endif()
set(PYTHON_INSTALL_PREFIX  "lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/${PYTHON_PACKAGE_PATH}"  CACHE STRING "Subdirectory to install Python modules in")


include(cmake/python.cmake)
add_subdirectory( python )
add_subdirectory( tests )
