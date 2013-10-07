add_subdirectory( enums )

SET(PYTHON_SOURCES
    __init__.py )    

add_python_package("Python ert.analysis" ${PYTHON_INSTALL_PREFIX}/ert/analysis  "${PYTHON_SOURCES}" True)
