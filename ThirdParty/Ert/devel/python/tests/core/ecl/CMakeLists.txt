set(NFS_RUNPATH "" CACHE STRING  "Disk area which is shared among cluster nodes and can be used as CWD for LSF/RSH jobs.")
set(RSH_SERVERS "" CACHE STRING  "List of nodes which will be used to test the RSH driver")

set(TEST_SOURCES
    __init__.py
    test_deprecation.py
    test_ecl_3dkw.py
    test_ecl_default.py
    test_ecl_file.py
    test_ecl_init_file.py
    test_ecl_restart_file.py
    test_ecl_submit.py
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
    test_kw.py
    test_kw_function.py
    test_layer.py
    test_legacy_ecl.py
    test_npv.py
    test_region.py
    test_restart.py
    test_rft.py
    test_rft_cell.py
    test_statoil_faults.py
    test_sum.py
)

add_python_package("python.tests.ecl"  ${PYTHON_INSTALL_PREFIX}/tests/ecl "${TEST_SOURCES}" False)

addPythonTest(ecl.ecl_default ecl.test_ecl_default.EclDefaultTest )
addPythonTest(ecl.ecl_file ecl.test_ecl_file.EclFileTest LABELS StatoilData)
addPythonTest(ecl.ecl_queue_LOCAL ecl.test_ecl_submit.LocalSubmitTest LABELS StatoilData:Slow)
addPythonTest(ecl.fortio ecl.test_fortio.FortIOTest LABELS StatoilData)
addPythonTest(ecl.ecl_grdecl ecl.test_grdecl.GRDECLTest LABELS StatoilData)
addPythonTest(ecl.ecl_grid_statoil ecl.test_grid_statoil.GridTest LABELS StatoilData:Slow)
addPythonTest(ecl.ecl_grid ecl.test_grid.GridTest )
addPythonTest(ecl.ecl_kw ecl.test_kw.KWTest LABELS StatoilData)
addPythonTest(ecl.ecl_kw_function ecl.test_kw_function.KWFunctionTest)
addPythonTest(ecl.ecl_3dkw ecl.test_ecl_3dkw.Ecl3DKWTest )
addPythonTest(ecl.ecl_init_file ecl.test_ecl_init_file.InitFileTest LABELS StatoilData)
addPythonTest(ecl.ecl_restart_file ecl.test_ecl_restart_file.RestartFileTest LABELS StatoilData)
addPythonTest(ecl.legacy_ecl ecl.test_legacy_ecl.LegacyEclTest)
addPythonTest(ecl.ecl_restart ecl.test_restart.RestartTest LABELS StatoilData )
addPythonTest(ecl.ecl_region ecl.test_region.RegionTest LABELS StatoilData)
addPythonTest(ecl.ecl_rft ecl.test_rft.RFTTest LABELS StatoilData)
addPythonTest(ecl.ecl_rft_cell ecl.test_rft_cell.RFTCellTest)
addPythonTest(ecl.ecl_sum ecl.test_sum.SumTest LABELS StatoilData)
addPythonTest(ecl.ecl_sum_vector ecl.test_ecl_sum_vector.EclSumVectorTest LABELS StatoilData)
addPythonTest(ecl.ecl_sum_time_range ecl.test_ecl_sum.EclSumTest LABELS StatoilData)
addPythonTest(ecl.layer ecl.test_layer.LayerTest )
addPythonTest(ecl.faults ecl.test_faults.FaultTest )
addPythonTest(ecl.statoil_faults ecl.test_statoil_faults.StatoilFaultTest LABELS StatoilData)
addPythonTest(ecl.fault_blocks ecl.test_fault_blocks.FaultBlockTest )
addPythonTest(ecl.fault_blocks_statoil ecl.test_fault_blocks_statoil.FaultBlockTest LABELS StatoilData)
addPythonTest(ecl.ecl_npv ecl.test_npv.NPVTest LABELS StatoilData)
addPythonTest(ecl.ecl_deprecation ecl.test_deprecation.DeprecationTest )
addPythonTest(ecl.indexed_read ecl.test_indexed_read.EclIndexedReadTest LABELS StatoilData)


if (NOT ${NFS_RUNPATH} STREQUAL "")
    if (ERT_LSF_SUBMIT_TEST)
        addPythonTest(ert.ecl.ecl_queue_LSF tests.ecl.test_ecl_submit.LSFSubmitTest ARGUMENTS ${NFS_RUNPATH} LABELS StatoilData:Slow)
    endif()


    if (NOT ${RSH_SERVERS} STREQUAL "")
        addPythonTest(ert.ecl.ecl_queue_RSH tests.ecl.test_ecl_submit.RSHSubmitTest ARGUMENTS ${NFS_RUNPATH} ${RSH_SERVERS} LABELS StatoilData:Slow)
    endif()
endif()