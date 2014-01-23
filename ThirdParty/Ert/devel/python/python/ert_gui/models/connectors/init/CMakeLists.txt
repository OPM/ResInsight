set(PYTHON_SOURCES
    __init__.py
    case_list.py
    case_selector.py
    init_from_existing.py
    init_from_scratch.py
    init_history_length.py
    init_members.py
    init_parameters.py
    initialized_case_selector.py
    is_case_initialized.py
)

add_python_package("python.ert_gui.models.connectors.init"  ${PYTHON_INSTALL_PREFIX}/ert_gui/models/connectors/init "${PYTHON_SOURCES}" True)

