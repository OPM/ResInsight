set(PYTHON_SOURCES
    __init__.py
    argument.py
    configuration_line.py
    keyword.py
    token.py
    validation_status.py
)

add_python_package("python.ert_gui.ide.keywords.data"  ${PYTHON_INSTALL_PREFIX}/ert_gui/ide/keywords/data "${PYTHON_SOURCES}" True)

