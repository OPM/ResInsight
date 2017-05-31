set(TEST_SOURCES
    __init__.py
    test_basecclass.py
    test_basecenum.py
    test_basecvalue.py
    test_metawrap.py
    test_cfile.py
)

add_python_package("python.tests.cwrap" ${PYTHON_INSTALL_PREFIX}/tests/cwrap "${TEST_SOURCES}" False)

addPythonTest(tests.cwrap.test_basecclass.BaseCClassTest)
addPythonTest(tests.cwrap.test_basecenum.BaseCEnumTest)
addPythonTest(tests.cwrap.test_basecvalue.BaseCValueTest)
addPythonTest(tests.cwrap.test_metawrap.MetaWrapTest)
addPythonTest(tests.cwrap.test_cfile.CFILETest)

