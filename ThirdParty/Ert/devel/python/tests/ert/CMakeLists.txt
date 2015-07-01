set(TEST_SOURCES
    __init__.py
)

add_python_package("python.tests.ert"  ${PYTHON_INSTALL_PREFIX}/tests/ert "${TEST_SOURCES}" False)

add_subdirectory(analysis)
add_subdirectory(config)
add_subdirectory(enkf)
add_subdirectory(job_queue)
add_subdirectory(run)
add_subdirectory(sched)
add_subdirectory(server)
add_subdirectory(testcase)
