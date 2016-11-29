set(PYTHON_SOURCES
    __init__.py
    base_run_model.py
    ensemble_experiment.py
    ensemble_smoother.py
    iterated_ensemble_smoother.py
    multiple_data_assimilation.py
    simulations_tracker.py
)

add_python_package("python.ert_gui.simulation.models" ${PYTHON_INSTALL_PREFIX}/ert_gui/simulation/models "${PYTHON_SOURCES}" True)

