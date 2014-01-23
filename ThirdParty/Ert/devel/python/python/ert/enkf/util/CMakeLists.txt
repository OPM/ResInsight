set(PYTHON_SOURCES
    __init__.py
    time_map.py
)

add_python_package("python.ert.enkf.util"  ${PYTHON_INSTALL_PREFIX}/ert/enkf/util "${PYTHON_SOURCES}" True)

