set(PYTHON_SOURCES
    __init__.py
    analysis_module_variables_panel.py
    ensemble_experiment_panel.py
    ensemble_smoother_panel.py
    iterated_ensemble_smoother_panel.py
    multiple_data_assimilation_panel.py
    sensitivity_study_panel.py
    sensitivity_study_parameters_panel.py
    simulation_config_panel.py
    simulation_panel.py
)

add_python_package("python.ert_gui.simulation"  ${PYTHON_INSTALL_PREFIX}/ert_gui/simulation "${PYTHON_SOURCES}" True)

