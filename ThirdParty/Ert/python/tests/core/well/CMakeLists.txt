set(TEST_SOURCES
    __init__.py
    test_ecl_well.py
    test_ecl_well2.py
    test_ecl_well3.py
)

add_python_package("python.tests.core.well"  ${PYTHON_INSTALL_PREFIX}/tests/core/well "${TEST_SOURCES}" False)

if (STATOIL_TESTDATA_ROOT)
  addPythonTest(core.well.ecl_well core.well.test_ecl_well.EclWellTest LABELS StatoilData)
  addPythonTest(core.well.ecl_well2 core.well.test_ecl_well2.EclWellTest2 LABELS StatoilData)
  addPythonTest(core.well.ecl_well3 core.well.test_ecl_well3.EclWellTest3 LABELS StatoilData)
endif()
