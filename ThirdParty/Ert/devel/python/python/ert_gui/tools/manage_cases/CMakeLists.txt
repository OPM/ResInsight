set(PYTHON_SOURCES
    __init__.py
    case_init_configuration.py
    manage_cases_tool.py
)

add_python_package("python.ert_gui.tools.manage_cases"  ${PYTHON_INSTALL_PREFIX}/ert_gui/tools/manage_cases "${PYTHON_SOURCES}" True)

