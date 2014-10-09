set(PYTHON_SOURCES
    __init__.py
    ensemble_resizer.py
    ert_summary.py
)

add_python_package("python.ert_gui.models.connectors"  ${PYTHON_INSTALL_PREFIX}/ert_gui/models/connectors "${PYTHON_SOURCES}" True)

add_subdirectory(export)
add_subdirectory(init)
add_subdirectory(plot)
add_subdirectory(queue_system)
add_subdirectory(run)
add_subdirectory(load_results)
