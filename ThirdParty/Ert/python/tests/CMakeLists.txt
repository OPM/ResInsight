set(TEST_SOURCES
    __init__.py
)

add_python_package("python.tests" "${PYTHON_INSTALL_PREFIX}/tests" "${TEST_SOURCES}" False)
set(CTEST_PYTHONPATH ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX})

add_subdirectory(geometry)
add_subdirectory(util)
add_subdirectory(ecl)
add_subdirectory(well)
add_subdirectory(cwrap)
add_subdirectory(global)



