set(PYTHON_SOURCES
    __init__.py
    obs_vector.py
    summary_observation.py
)

add_python_package("python.ert.enkf.observations"  ${PYTHON_INSTALL_PREFIX}/ert/enkf/observations "${PYTHON_SOURCES}" True)

