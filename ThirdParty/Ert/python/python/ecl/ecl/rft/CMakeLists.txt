set(PYTHON_SOURCES
    __init__.py
    well_trajectory.py
)

add_python_package("python.ecl.ecl.rft"  ${PYTHON_INSTALL_PREFIX}/ecl/ecl/rft "${PYTHON_SOURCES}" True)
