set(PYTHON_SOURCES
    __init__.py
    case_selector.py
    ensemble_resizer.py
    run_path.py
    simulation_mode.py
)

add_python_package("Python ert_gui.models.connectors"  ${PYTHON_INSTALL_PREFIX}/ert_gui/models/connectors "${PYTHON_SOURCES}" True)

add_subdirectory(init)
add_subdirectory(queue_system)
add_subdirectory(run)
