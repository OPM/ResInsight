set(PYTHON_SOURCES
    __init__.py
    config_enums.py
    config_parser.py
)

add_python_package("Python ert.config" ${PYTHON_INSTALL_PREFIX}/ert/config "${PYTHON_SOURCES}" True)
