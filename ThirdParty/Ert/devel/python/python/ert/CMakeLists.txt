add_python_package("Python ert" ${PYTHON_INSTALL_PREFIX}/ert "__init__.py" T)

add_subdirectory( config )
add_subdirectory( cwrap )
add_subdirectory( ecl )
add_subdirectory( enkf )
add_subdirectory( geo )
add_subdirectory( job_queue )
add_subdirectory( rms )
add_subdirectory( sched )
add_subdirectory( util )
add_subdirectory( well )

