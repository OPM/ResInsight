# try import python module, if success, check its version, store as PY_module.
# the module is imported as-is, hence the case (e.g. PyQt4) must be correct.
function(find_python_package_version package)
  set(PY_VERSION_ACCESSOR "__version__")
  set(PY_package_name ${package})

  if(${package} MATCHES "PyQt5")
    set(PY_package_name "PyQt5.Qt")
    set(PY_VERSION_ACCESSOR "PYQT_VERSION_STR")
  endif()

  if(${package} MATCHES "PyQt4")
    set(PY_package_name "PyQt4.Qt")
    set(PY_VERSION_ACCESSOR "PYQT_VERSION_STR")
  endif()


  execute_process(COMMAND "${PYTHON_EXECUTABLE}" "-c" "import os.path; import inspect; import ${PY_package_name} as py_m; print(\"%s;%s\" % (py_m.${PY_VERSION_ACCESSOR} , os.path.dirname(os.path.dirname(inspect.getfile(py_m)))))"
                  RESULT_VARIABLE _${package}_fail#    error code 0 if success
                  OUTPUT_VARIABLE stdout_output # major.minor.patch
                  ERROR_VARIABLE stderr_output
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(NOT _${package}_fail)
    list(GET stdout_output 0 version)
    set(PY_${package} ${version})  # local scope, for message
    set(PY_${package} ${version}     PARENT_SCOPE)

    list(GET stdout_output 1 path)
    set(PY_${package}_PATH ${path})  # local scope, for message
    set(PY_${package}_PATH ${path}     PARENT_SCOPE)
  endif()
endfunction()



# If we find the correct module and new enough version, set PY_package, where
# "package" is the given argument to the version we found else, display warning
# and do not set any variables.
function(find_python_package package version python_prefix)

  if (CMAKE_PREFIX_PATH)
     set( ORG_PYTHONPATH $ENV{PYTHONPATH} )
     set( CMAKE_PYTHONATH "")
     set( sep "")
     foreach ( PREFIX_PATH ${CMAKE_PREFIX_PATH} )
         set( CMAKE_PYTHONPATH "${CMAKE_PYTHONPATH}${sep}${PREFIX_PATH}/${python_prefix}")
         set( sep ":")
     endforeach()

     set(ENV{PYTHONPATH} "${CMAKE_PYTHONPATH}:${ORG_PYTHONPATH}")

     find_python_package_version(${package})
     if (DEFINED PY_${package})
        STRING(FIND ${CMAKE_PYTHONPATH} ${PY_${package}_PATH} pos)
        if (${pos} GREATER -1)
           set(CTEST_PYTHONPATH "${PY_${package}_PATH}:${CMAKE_PYTHONPATH}" PARENT_SCOPE)
        endif()
     endif()
     set(ENV{PYTHONPATH} ${ORG_PYTHONPATH})
  else()
     find_python_package_version(${package})
  endif()

  if(NOT DEFINED PY_${package})
     message("Could not find Python package " ${package})
  elseif(${PY_${package}} VERSION_LESS ${version})
    message(WARNING "Python package ${package} too old.  "
      "Wanted ${version}, found ${PY_${package}}")
  else()
    message(STATUS "Found ${package}.  ${PY_${package}} >= ${version} in ${PY_${package}_PATH}")
    set(PY_${package} ${version} PARENT_SCOPE)
    set(PY_${package}_PATH ${PY_${package}_PATH} PARENT_SCOPE)
  endif()
endfunction()
