set(PYTHON_SOURCES
    __init__.py
    analysis_module_options_enum.py
    analysis_module_load_status_enum.py
)

add_python_package("python.ert.analysis.enums"  ${PYTHON_INSTALL_PREFIX}/ert/analysis/enums "${PYTHON_SOURCES}" True)



