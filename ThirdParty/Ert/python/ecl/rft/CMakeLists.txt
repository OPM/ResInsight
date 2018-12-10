set(PYTHON_SOURCES
    __init__.py
    well_trajectory.py
    ecl_rft.py
    ecl_rft_cell.py
)

add_python_package("python.ecl.rft"  ${PYTHON_INSTALL_PREFIX}/ecl/rft "${PYTHON_SOURCES}" True)
