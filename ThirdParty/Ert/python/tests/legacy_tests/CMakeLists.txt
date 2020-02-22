set(TEST_SOURCES
    __init__.py
    test_ecl.py
    test_geo.py
    test_well.py
    test_util.py
    test_test.py
)

add_python_package("python.tests.legacy_tests.ert"  ${PYTHON_INSTALL_PREFIX}/tests/legacy_tests "${TEST_SOURCES}" False)

addPythonTest(tests.legacy_tests.test_ecl.ErtLegacyEclTest)
addPythonTest(tests.legacy_tests.test_geo.ErtLegacyGeoTest)
addPythonTest(tests.legacy_tests.test_well.ErtLegacyWellTest)
addPythonTest(tests.legacy_tests.test_util.ErtLegacyUtilTest)
addPythonTest(tests.legacy_tests.test_test.ErtLegacyTestTest)
