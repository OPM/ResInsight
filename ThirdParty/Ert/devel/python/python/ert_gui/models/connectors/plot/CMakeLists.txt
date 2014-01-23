set(PYTHON_SOURCES
    __init__.py
    data_type_keys_model.py
    ensemble_summary_plot.py
)

add_python_package("python.ert_gui.models.connectors.plot"  ${PYTHON_INSTALL_PREFIX}/ert_gui/models/connectors/plot "${PYTHON_SOURCES}" True)

