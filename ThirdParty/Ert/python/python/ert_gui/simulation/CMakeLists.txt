set(PYTHON_SOURCES
    __init__.py
    ensemble_experiment_panel.py
    ensemble_smoother_panel.py
    iterated_ensemble_smoother_panel.py
    multiple_data_assimilation_panel.py
    progress.py
    run_dialog.py
    simple_progress.py
    simulation_config_panel.py
    simulation_panel.py
)

add_python_package("python.ert_gui.simulation" ${PYTHON_INSTALL_PREFIX}/ert_gui/simulation "${PYTHON_SOURCES}" True)

add_subdirectory(models)
