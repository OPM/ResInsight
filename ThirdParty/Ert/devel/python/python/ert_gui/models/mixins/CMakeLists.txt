set(PYTHON_SOURCES
    __init__.py
    basic_model.py
    boolean_model.py
    button_model.py
    choice_model.py
    dictionary_model.py
    keyword_model.py
    list_model.py
    model.py
    path_model.py
    range_model.py
    run_model.py
    selectable_list_model.py
    selectable_model.py
    spinner_model.py
)

add_python_package("python.ert_gui.models.mixins"  ${PYTHON_INSTALL_PREFIX}/ert_gui/models/mixins "${PYTHON_SOURCES}" True)

add_subdirectory(connectorless)
