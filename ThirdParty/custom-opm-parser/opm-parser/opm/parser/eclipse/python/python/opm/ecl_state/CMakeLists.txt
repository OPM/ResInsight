set(PYTHON_SOURCES
    __init__.py
)

add_python_package("opm.ecl_state"  ${PYTHON_INSTALL_PREFIX}/opm/ecl_state "${PYTHON_SOURCES}" True)

add_subdirectory( tables )
add_subdirectory( schedule )
add_subdirectory( grid )
