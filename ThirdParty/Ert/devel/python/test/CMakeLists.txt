set(NFS_RUNPATH "" CACHE STRING  "Disk area which is shared among cluster nodes and can be used as CWD for LSF/RSH jobs.")
set(RSH_SERVERS "" CACHE STRING  "List of nodes which will be used to test the RSH driver")

set(TEST_SOURCES
    ert_tests/__init__.py
    ert_tests/source_enumerator.py
    ert_tests/run_tests.py
    ert_tests/test_ert_import.py

    ert_tests/config/__init__.py
    ert_tests/config/test_config.py

    ert_tests/cwrap/__init__.py
    ert_tests/cwrap/test_cwrap.py

    ert_tests/ecl/__init__.py
    ert_tests/ecl/test_ecl_default.py
    ert_tests/ecl/test_ecl_file.py
    ert_tests/ecl/test_ecl_submit.py
    ert_tests/ecl/test_fortio.py
    ert_tests/ecl/test_grdecl.py
    ert_tests/ecl/test_grid.py
    ert_tests/ecl/test_kw.py
    ert_tests/ecl/test_legacy_ecl.py
    ert_tests/ecl/test_region.py
    ert_tests/ecl/test_restart.py
    ert_tests/ecl/test_rft.py
    ert_tests/ecl/test_rft_cell.py
    ert_tests/ecl/test_sum.py

    ert_tests/enkf/__init__.py
    ert_tests/enkf/test_enkf.py
    ert_tests/enkf/test_enkf_library.py
    ert_tests/enkf/test_analysis_config.py

    ert_tests/job_queue/__init__.py
    ert_tests/job_queue/test_job_queue.py

    ert_tests/sched/__init__.py
    ert_tests/sched/test_sched.py

    ert_tests/util/__init__.py
    ert_tests/util/test_ctime.py
    ert_tests/util/test_hash.py
    ert_tests/util/test_lookup_table.py
    ert_tests/util/test_matrix.py
    ert_tests/util/test_stat.py
    ert_tests/util/test_string_list.py
    ert_tests/util/test_vectors.py

    ert_tests/run/__init__.py
    ert_tests/run/test_run.py

    ecl_isosurf.py
    ens_config_test.py
    grav_test.py
    grid_test0.py
    job_test.py
    petrel_kw.py
    poly_test.py
    troll_test.py
    ctest_run.py
)

if (ERT_BUILD_GUI) 
   LIST(APPEND ${TEST_SOURCES} ert_tests/test_ert_gui_import.py)
endif()

add_python_package("Python test" "${PYTHON_INSTALL_PREFIX}/test" "${TEST_SOURCES}" False)

# The test data is located in the current source directory; that is
# the reason we set that as the working directory for the test
# run. The module to import should then be in the default python
# module search path (i.e. current directory), whereas the location of
# the actual ert package is given by the first argument to ctest.

add_test( NAME python.ert.import_all_ert
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.test_ert_import.ErtImportTest )


if (ERT_BUILD_GUI) 
   add_test( NAME python.ert.import_all_ert_gui
             WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.test_ert_gui_import.ErtGuiImportTest )
endif()


#UTIL

add_test( NAME python.ert.util.stringlist
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.util.test_string_list.StringListTest )

add_test( NAME python.ert.util.tvector
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.util.test_vectors.UtilTest )

add_test( NAME python.ert.util.ctime
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.util.test_ctime.CTimeTest)


add_test( NAME python.ert.util.hash
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.util.test_hash.HashTest)


#add_test( NAME python.ert.util.latex
#          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.util.test_latex.LatexTest )

add_test( NAME python.ert.util.lookup_table
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.util.test_lookup_table.LookupTableTest )

add_test( NAME python.ert.util.matrix
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.util.test_matrix.MatrixTest )

add_test( NAME python.ert.util.stat
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.util.test_stat.StatTest )


#ENKF
add_test( NAME python.ert.enkf
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.enkf.test_enkf.EnKFTest)

add_test( NAME python.ert.enkf_library
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.enkf.test_enkf_library.EnKFLibraryTest)

add_test( NAME python.ert.enkf_analysis_config
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.enkf.test_analysis_config.AnalysisConfigTest)


#GUI
if (ERT_BUILD_GUI) 
   add_test( NAME python.ert_gui.models
             WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.gui.test_model.ModelTest)

   add_test( NAME python.ert_gui.models.observable
             WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.gui.test_observable.ObservableTest)
endif()

#ECL


add_test( NAME python.ert.ecl.ecl_default
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_ecl_default.EclDefaultTest )

add_test( NAME python.ert.ecl.ecl_file
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_ecl_file.FileTest)

add_test( NAME python.ert.ecl.ecl_queue_LOCAL
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_ecl_submit.LocalSubmitTest  )

if (NOT ${NFS_RUNPATH} STREQUAL "")
   add_test( NAME python.ert.ecl.ecl_queue_LSF
             WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_ecl_submit.LSFSubmitTest ${NFS_RUNPATH} )
   set_property( TEST python.ert.ecl.ecl_queue_LSF     PROPERTY LABELS Python:StatoilData:Slow )

   if (NOT ${RSH_SERVERS} STREQUAL "")
      add_test( NAME python.ert.ecl.ecl_queue_RSH
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_ecl_submit.RSHSubmitTest ${NFS_RUNPATH} ${RSH_SERVERS})
      set_property( TEST python.ert.ecl.ecl_queue_RSH     PROPERTY LABELS Python:StatoilData:Slow )
   endif()
endif()


add_test( NAME python.ert.ecl.fortio
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_fortio.FortIOTest)


add_test( NAME python.ert.ecl.ecl_grdecl                                      
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_grdecl.GRDECLTest )

add_test( NAME python.ert.ecl.ecl_grid      
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_grid.GridTest )

add_test( NAME python.ert.ecl.ecl_kw        
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_kw.KWTest )

add_test( NAME python.ert.ecl.legacy_ecl
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_legacy_ecl.LegacyEclTest)

add_test( NAME python.ert.ecl.ecl_restart
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_restart.RestartTest)

add_test( NAME python.ert.ecl.ecl_region
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_region.RegionTest)

add_test( NAME python.ert.ecl.ecl_rft 
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_rft.RFTTest)

add_test( NAME python.ert.ecl.ecl_rft_cell 
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_rft_cell.RFTCellTest)

add_test( NAME python.ert.ecl.ecl_sum
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_sum.SumTest)




#SCHED
add_test( NAME python.ert.sched.sched
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.sched.test_sched.SchedFileTest)

#JOB QUEUE
add_test( NAME python.ert.job_queue.job_status_type_enum
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.job_queue.test_job_queue.JobQueueTest)


#CONFIG
add_test( NAME python.ert.config
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.config.test_config.ConfigTest)

#CWRAP
add_test( NAME python.ert.cwrap
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.cwrap.test_cwrap.CWRapTest)

add_test( NAME python.ert.cwrap.basecclass
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.cwrap.test_basecclass.BaseCClassTest)

add_test( NAME python.ert.cwrap.basecenum
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.cwrap.test_basecenum.BaseCEnumTest)

#TESTRUN
add_test( NAME python.ert.test_run
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.run.test_run.RunTest )

#ANALYSIS
add_test( NAME python.ert.analysis.enums
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.analysis.test_options_enum.AnalysisOptionsEnumTest)




set_property( TEST python.ert.sched.sched           PROPERTY LABELS Python:StatoilData )
set_property( TEST python.ert.ecl.fortio            PROPERTY LABELS Python:StatoilData )
set_property( TEST python.ert.ecl.ecl_grdecl        PROPERTY LABELS Python:StatoilData )
set_property( TEST python.ert.ecl.ecl_restart       PROPERTY LABELS Python:StatoilData )
set_property( TEST python.ert.ecl.ecl_region        PROPERTY LABELS Python:StatoilData )
set_property( TEST python.ert.ecl.ecl_file          PROPERTY LABELS Python:StatoilData )
set_property( TEST python.ert.ecl.ecl_queue_LOCAL   PROPERTY LABELS Python:StatoilData:Slow )
set_property( TEST python.ert.ecl.ecl_sum           PROPERTY LABELS Python:StatoilData )
set_property( TEST python.ert.ecl.ecl_kw            PROPERTY LABELS Python:StatoilData )
set_property( TEST python.ert.ecl.ecl_rft           PROPERTY LABELS Python:StatoilData )
set_property( TEST python.ert.ecl.ecl_grid          PROPERTY LABELS Python:StatoilData:Slow )
#set_property( TEST python.import_local              PROPERTY LABELS Python:StatoilBuild )

set_property( TEST python.ert.test_run              PROPERTY LABELS Python)
set_property( TEST python.ert.import_all_ert        PROPERTY LABELS Python)
set_property( TEST python.ert.util.stringlist       PROPERTY LABELS Python )
set_property( TEST python.ert.util.tvector          PROPERTY LABELS Python )

if (ERT_BUILD_GUI)
   set_property( TEST python.ert.import_all_ert_gui    PROPERTY ENVIRONMENT "ERT_SHARE_PATH=${PROJECT_SOURCE_PATH}/share")
   set_property( TEST python.ert.import_all_ert_gui    PROPERTY LABELS Python)
   set_property( TEST python.ert_gui.models            PROPERTY LABELS Python)
   set_property( TEST python.ert_gui.models.observable PROPERTY LABELS Python)
endif()
