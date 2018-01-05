set(TEST_SOURCES
    __init__.py
    test_ecl.py
    test_geo.py
    test_well.py
    test_util.py
    test_test.py    
)

add_python_package("python.tests.legacy.ert"  ${PYTHON_INSTALL_PREFIX}/tests/legacy "${TEST_SOURCES}" False)

addPythonTest(tests.legacy.test_ecl.ErtLegacyEclTest)
addPythonTest(tests.legacy.test_geo.ErtLegacyGeoTest)
addPythonTest(tests.legacy.test_well.ErtLegacyWellTest)
addPythonTest(tests.legacy.test_util.ErtLegacyUtilTest)
addPythonTest(tests.legacy.test_test.ErtLegacyTestTest)
