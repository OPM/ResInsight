set(PYTHON_SOURCES
    __init__.py
    enums.py
    erttypes.py
    ertwrapper.py
    c_enums.py
)

add_python_package( "Python ert.ert" ${PYTHON_INSTALL_PREFIX}/ert/ert "${PYTHON_SOURCES}" True)
