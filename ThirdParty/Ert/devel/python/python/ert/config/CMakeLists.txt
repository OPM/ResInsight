set(PYTHON_SOURCES
    __init__.py
    config_error.py
    config_parser.py
    config_content.py
    content_type_enum.py
    unrecognized_enum.py
)

add_python_package("python.ert.config"  ${PYTHON_INSTALL_PREFIX}/ert/config "${PYTHON_SOURCES}" True)

