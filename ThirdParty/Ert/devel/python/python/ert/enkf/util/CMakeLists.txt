set(PYTHON_SOURCES
    __init__.py
    obs_vector.py
    time_map.py
)

add_python_package("Python ert.enkf.util" ${PYTHON_INSTALL_PREFIX}/ert/enkf/util "${PYTHON_SOURCES}" True)

