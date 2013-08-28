set(NFS_RUNPATH "" CACHE STRING  "Disk area which is shared among cluster nodes and can be used as CWD for LSF/RSH jobs.")
set(RSH_SERVERS "" CACHE STRING  "List of nodes which will be used to test the RSH driver")

set(TEST_SOURCES
    ert_tests/__init__.py
    ert_tests/run_tests.py
    ert_tests/test_ert_import.py
    ert_tests/test_ert_gui_import.py

    ert_tests/config/__init__.py
    ert_tests/config/test_config.py

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

    ert_tests/sched/__init__.py
    ert_tests/sched/test_sched.py

    ert_tests/util/__init__.py
    ert_tests/util/test_ctime.py
    ert_tests/util/test_lookup_table.py
    ert_tests/util/test_matrix.py
    ert_tests/util/test_stat.py
    ert_tests/util/test_string_list.py
    ert_tests/util/test_vectors.py

    ecl_isosurf.py
    enkf_test.py
    ens_config_test.py
    grav_test.py
    grid_test0.py
    job_test.py
    petrel_kw.py
    poly_test.py
    troll_test.py
    ctest_run.py
)


add_python_package("Python test" "${PYTHON_INSTALL_PREFIX}/test" "${TEST_SOURCES}" True)

# The test data is located in the current source directory; that is
# the reason we set that as the working directory for the test
# run. The module to import should then be in the default python
# module search path (i.e. current directory), whereas the location of
# the actual ert package is given by the first argument to ctest.

add_test( NAME python.ert.import_all_ert
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.test_ert_import.ErtImportTest )

add_test( NAME python.ert.import_all_ert_gui
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.test_ert_gui_import.ErtGuiImportTest )


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

add_test( NAME python.ert.util.latex
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.util.test_latex.LatexTest )

add_test( NAME python.ert.util.lookup_table
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.util.test_lookup_table.LookupTableTest )

add_test( NAME python.ert.util.matrix
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.util.test_matrix.MatrixTest )

add_test( NAME python.ert.util.stat
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.util.test_stat.StatTest )


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

#CONFIG
add_test( NAME python.ert.config
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.config.test_config.ConfigTest)




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

set_property( TEST python.ert.import_all_ert            PROPERTY LABELS Python)
set_property( TEST python.ert.import_all_ert_gui        PROPERTY LABELS Python)
set_property( TEST python.ert.util.stringlist       PROPERTY LABELS Python )
set_property( TEST python.ert.util.tvector          PROPERTY LABELS Python )
#set_property( TEST python.ert.import_all_ert            PROPERTY ENVIRONMENT "ERT_SHARE_PATH=${PROJECT_SOURCE_PATH}/share")
set_property( TEST python.ert.import_all_ert_gui        PROPERTY ENVIRONMENT "ERT_SHARE_PATH=${PROJECT_SOURCE_PATH}/share")

