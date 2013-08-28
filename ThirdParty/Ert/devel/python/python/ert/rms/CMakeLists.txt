set(PYTHON_SOURCES
    __init__.py
    librms.py
    rms.py

)

add_python_package("Python ert.rms" ${PYTHON_INSTALL_PREFIX}/ert/rms "${PYTHON_SOURCES}" True)
