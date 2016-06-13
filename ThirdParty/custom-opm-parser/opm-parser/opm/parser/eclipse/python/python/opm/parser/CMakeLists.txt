set(PYTHON_SOURCES
    __init__.py
    parser.py
    parse_context.py
    error_action.py
)

add_python_package("opm.parser"  ${PYTHON_INSTALL_PREFIX}/opm/parser "${PYTHON_SOURCES}" True)
