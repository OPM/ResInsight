set(PYTHON_SOURCES
    __init__.py
    cases.py
    custom_kw_keys.py
    debug.py
    ertshell.py
    gen_data_keys.py
    gen_kw_keys.py
    plot_settings.py
    plugins.py
    results.py
    shell_context.py
    shell_function.py
    shell_plot.py
    shell_tools.py
    simulations.py
    summary_keys.py
    workflows.py
)

add_python_package("python.ert_gui.shell"  ${PYTHON_INSTALL_PREFIX}/ert_gui/shell "${PYTHON_SOURCES}" True)

