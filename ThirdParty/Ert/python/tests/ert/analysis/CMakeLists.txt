set(TEST_SOURCES
    __init__.py
    test_analysis_module.py
    test_linalg.py
    test_options_enum.py
    test_rml.py
    test_std_enkf.py
    test_std_enkf_debug.py
)

add_python_package("python.tests.ert.analysis" ${PYTHON_INSTALL_PREFIX}/tests/ert/analysis "${TEST_SOURCES}" False)

addPythonTest(ert.analysis.analysis_module tests.ert.analysis.test_analysis_module.AnalysisModuleTest)
addPythonTest(ert.analysis.enums tests.ert.analysis.test_options_enum.AnalysisOptionsEnumTest)
addPythonTest(ert.analysis.linalg tests.ert.analysis.test_linalg.LinalgTest)
addPythonTest(ert.analysis.rml      tests.ert.analysis.test_rml.RMLTest)
addPythonTest(ert.analysis.std_enkf tests.ert.analysis.test_std_enkf.StdEnKFTest)
addPythonTest(ert.analysis.std_enkf_debug tests.ert.analysis.test_std_enkf_debug.StdEnKFDebugTest)

