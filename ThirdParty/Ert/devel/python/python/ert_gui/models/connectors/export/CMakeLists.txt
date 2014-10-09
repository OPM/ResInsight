set(PYTHON_SOURCES
    __init__.py
    export_keyword_model.py
    export_model.py
)

add_python_package("python.ert_gui.models.connectors.export"  ${PYTHON_INSTALL_PREFIX}/ert_gui/models/connectors/export "${PYTHON_SOURCES}" True)

