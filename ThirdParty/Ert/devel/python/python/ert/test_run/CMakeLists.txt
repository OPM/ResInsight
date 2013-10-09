SET(PYTHON_SOURCES
    __init__.py
    test_run.py
 )

add_python_package("Python ert.test_run"  ${PYTHON_INSTALL_PREFIX}/ert/test_run  "${PYTHON_SOURCES}" True)


