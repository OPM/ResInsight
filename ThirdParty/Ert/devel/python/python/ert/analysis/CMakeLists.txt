set(PYTHON_SOURCES
    __init__.py
    analysis_module.py
)

add_python_package("python.ert.analysis."  ${PYTHON_INSTALL_PREFIX}/ert/analysis/ "${PYTHON_SOURCES}" True)

add_subdirectory(enums)
