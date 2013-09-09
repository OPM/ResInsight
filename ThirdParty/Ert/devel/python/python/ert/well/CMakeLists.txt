set(PYTHON_SOURCES
    __init__.py
    libwell.py
    well_info.py
    well.py
    well_state.py
    well_ts.py
)

add_python_package("Python ert.well"  ${PYTHON_INSTALL_PREFIX}/ert/well "${PYTHON_SOURCES}" True)
