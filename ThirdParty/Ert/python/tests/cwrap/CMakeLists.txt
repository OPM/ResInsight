set(TEST_SOURCES
    __init__.py
    test_basecclass.py
    test_basecenum.py
    test_basecvalue.py
    test_metawrap.py
    test_cfile.py
)

add_python_package("python.tests.cwrap" ${PYTHON_INSTALL_PREFIX}/tests/cwrap "${TEST_SOURCES}" False)

addPythonTest(cwrap.basecclass tests.cwrap.test_basecclass.BaseCClassTest)
addPythonTest(cwrap.basecenum tests.cwrap.test_basecenum.BaseCEnumTest)
addPythonTest(cwrap.basecvalue tests.cwrap.test_basecvalue.BaseCValueTest)
addPythonTest(cwrap.metacwrap tests.cwrap.test_metawrap.MetaWrapTest)
addPythonTest(cwrap.cfile tests.cwrap.test_cfile.CFILETest)

