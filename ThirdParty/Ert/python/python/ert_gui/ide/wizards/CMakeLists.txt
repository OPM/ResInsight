set(PYTHON_SOURCES
    __init__.py
    tree_item.py
    tree_model.py
    wizard_view.py
)

add_python_package("python.ert_gui.ide.wizards"  ${PYTHON_INSTALL_PREFIX}/ert_gui/ide/wizards "${PYTHON_SOURCES}" True)

