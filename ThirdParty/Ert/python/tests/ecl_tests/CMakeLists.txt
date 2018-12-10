set(TEST_SOURCES
    __init__.py
    test_deprecation.py
    test_ecl_ecl.py
    test_removed.py
    test_ecl_3dkw.py
    test_ecl_file_statoil.py
    test_ecl_file.py
    test_ecl_init_file.py
    test_ecl_restart_file.py
    test_debug.py
    test_ecl_sum.py
    test_ecl_sum_vector.py
    test_fault_blocks.py
    test_fault_blocks_statoil.py
    test_faults.py
    test_fortio.py
    test_grdecl_statoil.py
    test_grdecl.py
    test_grid.py
    test_grid_pandas.py
    test_cell.py
    test_grid_statoil.py
    test_grid_statoil_coarse.py
    test_grid_statoil_dual.py
    test_grid_statoil_large_case.py
    test_grid_generator.py
    test_indexed_read.py
    test_ecl_kw_statoil.py
    test_ecl_kw.py
    test_kw_function.py
    test_layer.py
    test_npv.py
    test_region.py
    test_region_statoil.py
    test_restart.py
    test_rft.py
    test_rft_statoil.py
    test_rft_cell.py
    test_statoil_faults.py
    test_sum_statoil.py
    test_ecl_util.py
    test_ecl_cmp.py
    test_sum.py
    test_grav.py
    test_geertsma.py
    test_ecl_type.py
    test_restart_head.py
    test_fk_user_data.py
)

add_python_package("python.tests.ecl_tests"  ${PYTHON_INSTALL_PREFIX}/tests/ecl_tests "${TEST_SOURCES}" False)

addPythonTest(tests.ecl_tests.test_fk_user_data.FKTest)
addPythonTest(tests.ecl_tests.test_cell.CellTest)
addPythonTest(tests.ecl_tests.test_grid.GridTest LABELS SLOW_1)
addPythonTest(tests.ecl_tests.test_grid_pandas.GridPandasTest)
addPythonTest(tests.ecl_tests.test_grid_generator.GridGeneratorTest LABELS SLOW_2)
addPythonTest(tests.ecl_tests.test_ecl_kw.KWTest LABELS SLOW_2)
addPythonTest(tests.ecl_tests.test_kw_function.KWFunctionTest)
addPythonTest(tests.ecl_tests.test_ecl_3dkw.Ecl3DKWTest )
addPythonTest(tests.ecl_tests.test_rft.RFTTest)
addPythonTest(tests.ecl_tests.test_rft_cell.RFTCellTest)
addPythonTest(tests.ecl_tests.test_sum.SumTest)
addPythonTest(tests.ecl_tests.test_layer.LayerTest )
addPythonTest(tests.ecl_tests.test_faults.FaultTest )
addPythonTest(tests.ecl_tests.test_fault_blocks.FaultBlockTest )
addPythonTest(tests.ecl_tests.test_deprecation.Deprecation_1_9_Test )
addPythonTest(tests.ecl_tests.test_deprecation.Deprecation_2_0_Test )
addPythonTest(tests.ecl_tests.test_deprecation.Deprecation_2_1_Test )
addPythonTest(tests.ecl_tests.test_removed.Removed_2_1_Test )
addPythonTest(tests.ecl_tests.test_ecl_util.EclUtilTest )
addPythonTest(tests.ecl_tests.test_fortio.FortIOTest)
addPythonTest(tests.ecl_tests.test_ecl_file.EclFileTest)
addPythonTest(tests.ecl_tests.test_grav.EclGravTest)
addPythonTest(tests.ecl_tests.test_geertsma.GeertsmaTest)
addPythonTest(tests.ecl_tests.test_ecl_type.EclDataTypeTest)
addPythonTest(tests.ecl_tests.test_region.RegionTest)
addPythonTest(tests.ecl_tests.test_debug.DebugTest)

if (INSTALL_ERT_LEGACY)
  addPythonTest(tests.ecl_tests.test_ecl_ecl.EclEclTest)
endif()


addPythonTest(tests.ecl_tests.test_ecl_file_statoil.EclFileStatoilTest)
addPythonTest(tests.ecl_tests.test_grdecl_statoil.GRDECLStatoilTest)
addPythonTest(tests.ecl_tests.test_grdecl.GRDECLTest)
addPythonTest(tests.ecl_tests.test_grid_statoil.GridTest)
addPythonTest(tests.ecl_tests.test_grid_statoil_coarse.GridCoarceTest)
addPythonTest(tests.ecl_tests.test_grid_statoil_dual.GridDualTest)
addPythonTest(tests.ecl_tests.test_grid_statoil_large_case.GridLargeCaseTest)
addPythonTest(tests.ecl_tests.test_ecl_kw_statoil.KWTest)
addPythonTest(tests.ecl_tests.test_ecl_init_file.InitFileTest)
addPythonTest(tests.ecl_tests.test_ecl_restart_file.RestartFileTest)
addPythonTest(tests.ecl_tests.test_restart.RestartTest )
addPythonTest(tests.ecl_tests.test_region_statoil.RegionTest)
addPythonTest(tests.ecl_tests.test_rft_statoil.RFTTest)
addPythonTest(tests.ecl_tests.test_sum_statoil.SumTest)
addPythonTest(tests.ecl_tests.test_ecl_sum_vector.EclSumVectorTest)
addPythonTest(tests.ecl_tests.test_ecl_sum.EclSumTest)
addPythonTest(tests.ecl_tests.test_statoil_faults.StatoilFaultTest)
addPythonTest(tests.ecl_tests.test_fault_blocks_statoil.FaultBlockTest)
addPythonTest(tests.ecl_tests.test_npv.NPVTest)
addPythonTest(tests.ecl_tests.test_indexed_read.EclIndexedReadTest)
addPythonTest(tests.ecl_tests.test_ecl_cmp.EclCmpTest)
addPythonTest(tests.ecl_tests.test_restart_head.RestartHeadTest)
