set(PYTHON_SOURCES
    __init__.py
    ert_connector.py
    observable.py
    weak_method.py
)

add_python_package("Python ert_gui.models"  ${PYTHON_INSTALL_PREFIX}/ert_gui/models "${PYTHON_SOURCES}" True)

add_subdirectory(connectors)
add_subdirectory(mixins)
