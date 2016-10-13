set(PYTHON_SOURCES
    __init__.py
    schedule.py
)

add_python_package("opm.ecl_state.schedule"  ${PYTHON_INSTALL_PREFIX}/opm/ecl_state/schedule "${PYTHON_SOURCES}" True)
