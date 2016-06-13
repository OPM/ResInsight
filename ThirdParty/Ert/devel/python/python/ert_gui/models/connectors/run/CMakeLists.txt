set(PYTHON_SOURCES
    __init__.py
    active_realizations_model.py
    analysis_module_model.py
    analysis_module_variables_model.py
    base_run_model.py
    ensemble_experiment.py
    ensemble_smoother.py
    iterated_analysis_module_model.py
    iterated_ensemble_smoother.py
    multiple_data_assimilation.py
    number_of_iterations.py
    run_path.py
    sensitivity_study.py
    sensitivity_study_parameters_constant_value_model.py
    sensitivity_study_parameters_is_included_model.py
    sensitivity_study_parameters_model.py
    sensitivity_target_case_format_model.py
    simulation_mode.py
    simulations_tracker.py
    target_case_format_model.py
    target_case_model.py
    workflows_model.py
)

add_python_package("python.ert_gui.models.connectors.run"  ${PYTHON_INSTALL_PREFIX}/ert_gui/models/connectors/run "${PYTHON_SOURCES}" True)

