set(PYTHON_SOURCES
    __init__.py
    path_completer.py
)

add_python_package("python.ert_gui.ide.completers"  ${PYTHON_INSTALL_PREFIX}/ert_gui/ide/completers "${PYTHON_SOURCES}" True)

