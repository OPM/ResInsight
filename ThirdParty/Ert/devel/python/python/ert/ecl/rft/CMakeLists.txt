set(PYTHON_SOURCES
    __init__.py
    well_trajectory.py
)

add_python_package("python.ert.ecl.rft"  ${PYTHON_INSTALL_PREFIX}/ert/ecl/rft "${PYTHON_SOURCES}" True)
