set(PYTHON_SOURCES
    __init__.py
    ecl/__init__.py
    ecl/faults/__init__.py
    geo/__init__.py
    well/__init__.py
    util/__init__.py
    test/__init__.py
)
add_python_package("python.ert"  ${PYTHON_INSTALL_PREFIX}/ert "${PYTHON_SOURCES}" True)

