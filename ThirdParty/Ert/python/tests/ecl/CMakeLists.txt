set(NFS_RUNPATH "" CACHE STRING  "Disk area which is shared among cluster nodes and can be used as CWD for LSF/RSH jobs.")
set(RSH_SERVERS "" CACHE STRING  "List of nodes which will be used to test the RSH driver")

set(TEST_SOURCES
    __init__.py
    test_deprecation.py
    test_removed.py
    test_ecl_3dkw.py
    test_ecl_file_statoil.py
    test_ecl_file.py
    test_ecl_init_file.py
    test_ecl_restart_file.py
    test_ecl_sum.py
    test_ecl_sum_vector.py
    test_fault_blocks.py
    test_fault_blocks_statoil.py
    test_faults.py
    test_fortio.py
    test_grdecl.py
    test_grid.py
    test_grid_statoil.py
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
)

add_python_package("python.tests.ecl"  ${PYTHON_INSTALL_PREFIX}/tests/ecl "${TEST_SOURCES}" False)

addPythonTest(tests.ecl.test_grid.GridTest )
addPythonTest(tests.ecl.test_ecl_kw.KWTest)
addPythonTest(tests.ecl.test_kw_function.KWFunctionTest)
addPythonTest(tests.ecl.test_ecl_3dkw.Ecl3DKWTest )
addPythonTest(tests.ecl.test_rft.RFTTest)
addPythonTest(tests.ecl.test_rft_cell.RFTCellTest)
addPythonTest(tests.ecl.test_sum.SumTest)
addPythonTest(tests.ecl.test_layer.LayerTest )
addPythonTest(tests.ecl.test_faults.FaultTest )
addPythonTest(tests.ecl.test_fault_blocks.FaultBlockTest )
addPythonTest(tests.ecl.test_deprecation.Deprecation_1_9_Test )
addPythonTest(tests.ecl.test_deprecation.Deprecation_2_0_Test )
addPythonTest(tests.ecl.test_deprecation.Deprecation_2_1_Test )
addPythonTest(tests.ecl.test_removed.Removed_2_1_Test )
addPythonTest(tests.ecl.test_ecl_util.EclUtilTest )
addPythonTest(tests.ecl.test_fortio.FortIOTest)
addPythonTest(tests.ecl.test_ecl_file.EclFileTest)
addPythonTest(tests.ecl.test_grav.EclGravTest)
addPythonTest(tests.ecl.test_geertsma.GeertsmaTest)
addPythonTest(tests.ecl.test_ecl_type.EclDataTypeTest)
addPythonTest(tests.ecl.test_region.RegionTest)


if (STATOIL_TESTDATA_ROOT)
  addPythonTest(tests.ecl.test_ecl_file_statoil.EclFileStatoilTest LABELS StatoilData)
  addPythonTest(tests.ecl.test_grdecl.GRDECLTest LABELS StatoilData)
  addPythonTest(tests.ecl.test_grid_statoil.GridTest LABELS StatoilData:Slow)
  addPythonTest(tests.ecl.test_ecl_kw_statoil.KWTest LABELS StatoilData)
  addPythonTest(tests.ecl.test_ecl_init_file.InitFileTest LABELS StatoilData)
  addPythonTest(tests.ecl.test_ecl_restart_file.RestartFileTest LABELS StatoilData)
  addPythonTest(tests.ecl.test_restart.RestartTest LABELS StatoilData )
  addPythonTest(tests.ecl.test_region_statoil.RegionTest LABELS StatoilData)
  addPythonTest(tests.ecl.test_rft_statoil.RFTTest LABELS StatoilData)
  addPythonTest(tests.ecl.test_sum_statoil.SumTest LABELS StatoilData)
  addPythonTest(tests.ecl.test_ecl_sum_vector.EclSumVectorTest LABELS StatoilData)
  addPythonTest(tests.ecl.test_ecl_sum.EclSumTest LABELS StatoilData)
  addPythonTest(tests.ecl.test_statoil_faults.StatoilFaultTest LABELS StatoilData)
  addPythonTest(tests.ecl.test_fault_blocks_statoil.FaultBlockTest LABELS StatoilData)
  addPythonTest(tests.ecl.test_npv.NPVTest LABELS StatoilData)
  addPythonTest(tests.ecl.test_indexed_read.EclIndexedReadTest LABELS StatoilData)
  addPythonTest(tests.ecl.test_ecl_cmp.EclCmpTest LABELS StatoilData)
  addPythonTest(tests.ecl.test_restart_head.RestartHeadTest LABELS StatoilData)
endif()
