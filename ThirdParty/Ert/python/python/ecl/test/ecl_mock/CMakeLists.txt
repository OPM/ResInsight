set(PYTHON_SOURCES
    __init__.py
    ecl_sum_mock.py
)

add_python_package("python.ecl.test.ecl_mock"  ${PYTHON_INSTALL_PREFIX}/ecl/test/ecl_mock "${PYTHON_SOURCES}" True)
