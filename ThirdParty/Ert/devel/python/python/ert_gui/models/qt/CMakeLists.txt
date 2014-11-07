set(PYTHON_SOURCES
    __init__.py
    all_cases_model.py
)

add_python_package("python.ert_gui.models.qt"  ${PYTHON_INSTALL_PREFIX}/ert_gui/models/qt "${PYTHON_SOURCES}" True)


