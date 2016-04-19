set(TEST_SOURCES
    __init__.py
    test_arg_loader.py
    test_custom_kw_collector.py
    test_design_matrix.py
    test_export_join.py
    test_gen_data_collector.py
    test_gen_data_observation_collector.py
    test_gen_kw_collector.py
    test_misfit_collector.py
    test_numpy_and_pandas.py
    test_summary_collector.py
    test_summary_observation_collector.py
)

add_python_package("python.tests.ert.enkf.export" ${PYTHON_INSTALL_PREFIX}/tests/ert/enkf/export "${TEST_SOURCES}" False)

addPythonTest(ert.enkf.export.arg_loader tests.ert.enkf.export.test_arg_loader.ArgLoaderTest LABELS StatoilData)
addPythonTest(ert.enkf.export.custom_kw_collector tests.ert.enkf.export.test_custom_kw_collector.CustomKwCollectorTest)
addPythonTest(ert.enkf.export.design_matrix tests.ert.enkf.export.test_design_matrix.DesignMatrixTest)
addPythonTest(ert.enkf.export.export_join tests.ert.enkf.export.test_export_join.ExportJoinTest)
addPythonTest(ert.enkf.export.gen_data_collector tests.ert.enkf.export.test_gen_data_collector.GenDataCollectorTest)
addPythonTest(ert.enkf.export.gen_data_observation_collector tests.ert.enkf.export.test_gen_data_observation_collector.GenDataObservationCollectorTest)
addPythonTest(ert.enkf.export.gen_kw_collector tests.ert.enkf.export.test_gen_kw_collector.GenKwCollectorTest)
addPythonTest(ert.enkf.export.numpy_and_pandas tests.ert.enkf.export.test_numpy_and_pandas.NumpyAndPandasTest)
addPythonTest(ert.enkf.export.misfit_collector tests.ert.enkf.export.test_misfit_collector.MisfitCollectorTest)
addPythonTest(ert.enkf.export.summary_collector tests.ert.enkf.export.test_summary_collector.SummaryCollectorTest)
addPythonTest(ert.enkf.export.summary_observation_collector tests.ert.enkf.export.test_summary_observation_collector.SummaryObservationCollectorTest)
