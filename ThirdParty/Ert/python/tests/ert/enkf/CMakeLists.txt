set(TEST_SOURCES
    __init__.py
    test_active_list.py
    test_analysis_config.py
    test_analysis_iter_config.py
    test_block_obs.py
    test_custom_kw_config_set.py
    test_ecl_config.py
    test_es_update.py
    test_enkf.py
    test_enkf_fs.py
    test_enkf_fs_manager1.py
    test_enkf_fs_manager2.py
    test_enkf_library.py
    test_enkf_load_results_manually.py
    test_enkf_obs.py
    test_ert_context.py
    test_ert_log.py
    test_gen_obs.py
    test_labscale.py
    test_linalg.py
    test_local_obsdata_node.py
    test_local_config.py
    test_meas_block.py
    test_meas_data.py
    test_obs_block.py
    test_obs_data.py
    test_state_map.py
    test_summary_key_matcher.py
    test_summary_key_set.py
    test_summary_obs.py
    test_time_map.py
    test_update.py
    test_ensemble_config.py
    test_deprecation.py
    test_field_config.py
    test_field_export.py
    test_hook_workflow.py
    test_forward_load_context.py
    test_runpath_list.py
)

add_python_package("python.tests.ert.enkf" ${PYTHON_INSTALL_PREFIX}/tests/ert/enkf "${TEST_SOURCES}" False)
addPythonTest(ert.enkf.es_update tests.ert.enkf.test_es_update.ESUpdateTest)
addPythonTest(ert.enkf.forward_load_context tests.ert.enkf.test_forward_load_context.ForwardLoadContextTest)
addPythonTest(ert.enkf.deprecation tests.ert.enkf.test_deprecation.DeprecationTest)
addPythonTest(ert.enkf.meas_block tests.ert.enkf.test_meas_block.MeasBlockTest)
addPythonTest(ert.enkf.meas_data tests.ert.enkf.test_meas_data.MeasDataTest)
addPythonTest(ert.enkf.obs_data tests.ert.enkf.test_obs_data.ObsDataTest)
addPythonTest(ert.enkf.gen_obs tests.ert.enkf.test_gen_obs.GenObsTest)
addPythonTest(ert.enkf.summary_obs tests.ert.enkf.test_summary_obs.SummaryObsTest)
addPythonTest(ert.enkf.block_obs tests.ert.enkf.test_block_obs.BlockObsTest)
addPythonTest(ert.enkf.obs_block tests.ert.enkf.test_obs_block.ObsBlockTest)
addPythonTest(ert.enkf.ert_log tests.ert.enkf.test_ert_log.ErtLogTest)
addPythonTest(ert.enkf.analysis_config tests.ert.enkf.test_analysis_config.AnalysisConfigTest)
addPythonTest(ert.enkf.analysis_iter_config tests.ert.enkf.test_analysis_iter_config.AnalysisIterConfigTest)
addPythonTest(ert.enkf.state_map tests.ert.enkf.test_state_map.StateMapTest)
addPythonTest(ert.enkf.custom_kw_config_set tests.ert.enkf.test_custom_kw_config_set.CustomKWConfigSetTest)
addPythonTest(ert.enkf.summary_key_matcher tests.ert.enkf.test_summary_key_matcher.SummaryKeyMatcherTest)
addPythonTest(ert.enkf.time_map tests.ert.enkf.test_time_map.TimeMapTest)
addPythonTest(ert.enkf.active_list tests.ert.enkf.test_active_list.ActiveListTest)
addPythonTest(ert.enkf.local_obsdata_node tests.ert.enkf.test_local_obsdata_node.LocalObsdataNodeTest)
addPythonTest(ert.enkf.local_config tests.ert.enkf.test_local_config.LocalConfigTest)
addPythonTest(ert.enkf.ensemble_config tests.ert.enkf.test_ensemble_config.EnsembleConfigTest)
addPythonTest(ert.enkf.hook_workflow tests.ert.enkf.test_hook_workflow.HookWorkFlowTest)
addPythonTest(ert.enkf.runpath_list tests.ert.enkf.test_runpath_list.RunpathListTest)
addPythonTest(ert.enkf.field_config tests.ert.enkf.test_field_config.FieldConfigTest)

if (STATOIL_TESTDATA_ROOT)
  addPythonTest(ert.enkf.enkf tests.ert.enkf.test_enkf.EnKFTest LABELS StatoilData)
  addPythonTest(ert.enkf.enkf_obs tests.ert.enkf.test_enkf_obs.EnKFObsTest LABELS StatoilData)
  addPythonTest(ert.enkf.enkf_load_results_manually tests.ert.enkf.test_enkf_load_results_manually.LoadResultsManuallyTest LABELS StatoilData)
  addPythonTest(ert.enkf.enkf_fs tests.ert.enkf.test_enkf_fs.EnKFFSTest LABELS StatoilData)
  addPythonTest(ert.enkf.enkf_fs_manager1 tests.ert.enkf.test_enkf_fs_manager1.EnKFFSManagerTest1 LABELS StatoilData)
  addPythonTest(ert.enkf.enkf_fs_manager2 tests.ert.enkf.test_enkf_fs_manager2.EnKFFSManagerTest2 LABELS StatoilData)
  addPythonTest(ert.enkf.enkf_library tests.ert.enkf.test_enkf_library.EnKFLibraryTest)
  addPythonTest(ert.enkf.summary_key_set tests.ert.enkf.test_summary_key_set.SummaryKeySetTest LABELS StatoilData)
  addPythonTest(ert.enkf.ecl_config tests.ert.enkf.test_ecl_config.EclConfigTest LABELS StatoilData)
  addPythonTest(ert.enkf.ert_test_context tests.ert.enkf.test_ert_context.ErtTestContextTest LABELS StatoilData)
  addPythonTest(ert.enkf.update tests.ert.enkf.test_update.UpdateTest LABELS StatoilData)
  addPythonTest(ert.enkf.labscale tests.ert.enkf.test_labscale.LabScaleTest LABELS StatoilData)
  addPythonTest(ert.enkf.field_export tests.ert.enkf.test_field_export.FieldExportTest LABELS StatoilData)
endif()

add_subdirectory(data)
add_subdirectory(export)
add_subdirectory(plot)
