set(PYTHON_SOURCES
    __init__.py
    load_results_model.py
)

add_python_package("python.ert_gui.models.connectors.load_results"  ${PYTHON_INSTALL_PREFIX}/ert_gui/models/connectors/load_results "${PYTHON_SOURCES}" True)

