# - Find the Python wrappers for Ensemble-based Reservoir Tool (ERT)
#
# Set the cache variable ERT_PYTHON_PATH to the install location of
# the root ert package.  

find_package(PythonInterp)
if(PYTHONINTERP_FOUND)

# We try to find the ert Python distribution. This is done by running
# Python code which tries to 'import ert' and prints out the path to
# the module if the import succeeds.
#
# The normal Python import machinery is employed, so if you have
# installed ert python in a default location, or alternatively set the
# PYTHONPATH variable the ert Python distribution will eventually be
# found there, independently of the alternatives which are tested with
# the ${PATH_LIST} variable.

  if (EXISTS "/etc/debian_version")
     set( PYTHON_PACKAGE_PATH "dist-packages")
  else()
     set( PYTHON_PACKAGE_PATH "site-packages")
  endif()
  set(PYTHON_INSTALL_PREFIX  "lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/${PYTHON_PACKAGE_PATH}"  CACHE STRING "Subdirectory to install Python modules in")

  if (ERT_ROOT)
     set( start_path "${ERT_ROOT}/${PYTHON_INSTALL_PREFIX}" )
  else()
     set( start_path "DEFAULT_PATH")
  endif()

  set( PATH_LIST "${start_path}"
                 "${PROJECT_SOURCE_DIR}/../ert/build/${PYTHON_INSTALL_PREFIX}"
                 "${PROJECT_SOURCE_DIR}/../ert/devel/build/${PYTHON_INSTALL_PREFIX}"
                 "${PROJECT_BINARY_DIR}/../ert-build/${PYTHON_INSTALL_PREFIX}"
                 "${PROJECT_BINARY_DIR}/../ert/devel/${PYTHON_INSTALL_PREFIX}")

  foreach( PATH ${PATH_LIST})
      set( python_code "import sys; sys.path.insert(0 , '${PATH}'); import os.path; import inspect; import ert; print os.path.dirname(os.path.dirname(inspect.getfile(ert)))")
      execute_process( COMMAND ${PYTHON_EXECUTABLE} -c "${python_code}"
                       RESULT_VARIABLE import_result
                       OUTPUT_VARIABLE stdout_output
                       ERROR_VARIABLE stderr_output
                       OUTPUT_STRIP_TRAILING_WHITESPACE )

      if (${import_result} EQUAL 0)
         set( ERT_PYTHON_PATH ${stdout_output} CACHE PATH "Python path for ERT Python" )
         break()
      endif()
   endforeach()
endif()
find_package_handle_standard_args("ERTPython" DEFAULT_MSG ERT_PYTHON_PATH)



