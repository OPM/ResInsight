set(TEST_SOURCES
    __init__.py
    test_custom_kw.py
    test_custom_kw_config.py
    test_field_config.py
    test_gen_data.py
    test_gen_data_config.py
    test_gen_kw.py
    test_gen_kw_config.py
)

add_python_package("python.tests.ert.enkf.data" ${PYTHON_INSTALL_PREFIX}/tests/ert/enkf/data "${TEST_SOURCES}" False)

addPythonTest(ert.enkf.data.custom_kw tests.ert.enkf.data.test_custom_kw.CustomKWTest)
addPythonTest(ert.enkf.data.gen_kw tests.ert.enkf.data.test_gen_kw.GenKwTest)

if (STATOIL_TESTDATA_ROOT)
  addPythonTest(ert.enkf.data.gen_kw_config tests.ert.enkf.data.test_gen_kw_config.GenKwConfigTest LABELS StatoilData)
  addPythonTest(ert.enkf.data.gen_data tests.ert.enkf.data.test_gen_data.GenDataTest LABELS StatoilData)
  addPythonTest(ert.enkf.data.gen_data_config tests.ert.enkf.data.test_gen_data_config.GenDataConfigTest LABELS StatoilData)
endif()
