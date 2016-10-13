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

  set(PATH_LIST)
  if (ERT_ROOT)
      list(APPEND PATH_LIST ${ERT_ROOT})
  endif()
  list(APPEND PATH_LIST ${CMAKE_PREFIX_PATH})

  # Add various popular sibling alternatives.
  list(APPEND PATH_LIST "${PROJECT_SOURCE_DIR}/../ert/build"
    			"${PROJECT_BINARY_DIR}/../ert-build")

  foreach( PATH ${PATH_LIST})
      set( python_code "import sys; sys.path.insert(0 , '${PATH}/${PYTHON_INSTALL_PREFIX}'); import os.path; import inspect; import ert; print os.path.dirname(os.path.dirname(inspect.getfile(ert))); from ert.ecl import EclSum")
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



