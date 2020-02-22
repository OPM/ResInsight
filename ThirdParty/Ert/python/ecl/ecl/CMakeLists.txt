set(PYTHON_SOURCES
    __init__.py
)
add_python_package("python.ecl.ecl"  ${PYTHON_INSTALL_PREFIX}/ecl/ecl "${PYTHON_SOURCES}" True)

