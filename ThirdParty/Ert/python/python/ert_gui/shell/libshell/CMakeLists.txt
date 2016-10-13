set(PYTHON_SOURCES
    __init__.py
    shell_collection.py
    shell_context.py
    shell_function.py
    shell_path_tools.py
    shell_property.py
    shell_tools.py
)

add_python_package("python.ert_gui.shell.libshell" ${PYTHON_INSTALL_PREFIX}/ert_gui/shell/libshell "${PYTHON_SOURCES}" True)

