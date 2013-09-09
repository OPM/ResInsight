set(PYTHON_SOURCES
    __init__.py
    basecclass.py
    basecenum.py
    cclass.py
    cenum.py
    cfile.py
    clib.py
    cnamespace.py
    cwrap.py
)

add_python_package("Python ert.cwrap" ${PYTHON_INSTALL_PREFIX}/ert/cwrap "${PYTHON_SOURCES}" True)
