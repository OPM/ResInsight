set (TEST_SOURCES __init__.py )

add_python_package("python.tests.ecl_state" "${PYTHON_INSTALL_PREFIX}/tests/ecl_state" "${TEST_SOURCES}" False)

add_subdirectory( table )
add_subdirectory( grid )
add_subdirectory( schedule )
