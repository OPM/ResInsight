set(TEST_SOURCES
    __init__.py
    test_ecl_well.py
    test_ecl_well2.py
    test_ecl_well3.py
    test_well_missing_ICON.py
)

add_python_package("python.tests.well_tests"  ${PYTHON_INSTALL_PREFIX}/tests/well_tests "${TEST_SOURCES}" False)

addPythonTest(tests.well_tests.test_ecl_well.EclWellTest)
addPythonTest(tests.well_tests.test_ecl_well2.EclWellTest2)
addPythonTest(tests.well_tests.test_ecl_well3.EclWellTest3)
addPythonTest(tests.well_tests.test_well_missing_ICON.EclWellICONTest)
