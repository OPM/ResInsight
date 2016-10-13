set(TEST_SOURCES
    __init__.py
)

add_python_package("python.tests.core" ${PYTHON_INSTALL_PREFIX}/tests/core "${TEST_SOURCES}" False)

add_subdirectory(cwrap)
add_subdirectory(geometry)
add_subdirectory(util)
add_subdirectory(ecl)
add_subdirectory(well)
