set(PYTHON_SOURCES
    __init__.py
    analysis_module_keywords.py
    configuration_line_builder.py
    configuration_line_parser.py
    eclipse_keywords.py
    enkf_control_keywords.py
    ensemble_keywords.py
    ert_keywords.py
    parametrization_keywords.py
    plot_keywords.py
    queue_system_keywords.py
    run_keywords.py
    simulation_control_keywords.py
    workflow_keywords.py
    qc_keywords.py
    unix_environment_keywords.py
    advanced_keywords.py
)

add_python_package("python.ert_gui.ide.keywords"  ${PYTHON_INSTALL_PREFIX}/ert_gui/ide/keywords "${PYTHON_SOURCES}" True)

add_subdirectory(data)
add_subdirectory(definitions)
