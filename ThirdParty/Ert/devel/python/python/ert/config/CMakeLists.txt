set(PYTHON_SOURCES
    __init__.py
    content_type_enum.py
    config_parser.py
    unrecognized_enum.py
)

add_python_package("Python ert.config" ${PYTHON_INSTALL_PREFIX}/ert/config "${PYTHON_SOURCES}" True)
