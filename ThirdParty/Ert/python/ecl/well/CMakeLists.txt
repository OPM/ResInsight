set(PYTHON_SOURCES
    __init__.py
    well_connection.py
    well_connection_direction_enum.py
    well_info.py
    well_segment.py
    well_state.py
    well_time_line.py
    well_type_enum.py
)

add_python_package("python.ecl.well"  ${PYTHON_INSTALL_PREFIX}/ecl/well "${PYTHON_SOURCES}" True)

