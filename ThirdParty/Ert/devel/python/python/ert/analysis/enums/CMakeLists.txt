SET(PYTHON_SOURCES
    __init__.py 
    analysis_module_options_enum.py )

add_python_package("Python ert.analysis.enums" ${PYTHON_INSTALL_PREFIX}/ert/analysis/enums  "${PYTHON_SOURCES}" True)
