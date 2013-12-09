set(PYTHON_SOURCES
    __init__.py
)

add_python_package("python.ert"  ${PYTHON_INSTALL_PREFIX}/ert "${PYTHON_SOURCES}" True)

add_subdirectory(analysis)
add_subdirectory(config)
add_subdirectory(cwrap)
add_subdirectory(ecl)
add_subdirectory(enkf)
add_subdirectory(geo)
add_subdirectory(job_queue)
add_subdirectory(rms)
add_subdirectory(sched)
add_subdirectory(test_run)
add_subdirectory(util)
add_subdirectory(well)
