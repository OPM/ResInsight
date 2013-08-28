set(PYTHON_SOURCES
    __init__.py
    cclass.py
    cenum.py
    cfile.py
    clib.py
    cwrap.py
)

add_python_package("Python ert.cwrap" ${PYTHON_INSTALL_PREFIX}/ert/cwrap "${PYTHON_SOURCES}" True)
