set(TEST_SOURCES
    __init__.py
)

add_python_package("python.tests" "${PYTHON_INSTALL_PREFIX}/tests" "${TEST_SOURCES}" False)
add_subdirectory( parser )
add_subdirectory( deck )
add_subdirectory( parse_context )
add_subdirectory( ecl_state )
