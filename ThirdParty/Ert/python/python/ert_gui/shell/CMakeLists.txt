set(PYTHON_SOURCES
    __init__.py
    analysis_module.py
    cases.py
    custom_kw_keys.py
    debug.py
    export.py
    ertshell.py
    ert_shell_collection.py
    gen_data_keys.py
    gen_kw_keys.py
    observations.py
    plot_settings.py
    plugins.py
    results.py
    ert_shell_context.py
    server.py
    shell_plot.py
    simulations.py
    smoother.py
    storage.py
    summary_keys.py
    workflows.py
)

add_python_package("python.ert_gui.shell" ${PYTHON_INSTALL_PREFIX}/ert_gui/shell "${PYTHON_SOURCES}" True)

add_subdirectory(libshell)
