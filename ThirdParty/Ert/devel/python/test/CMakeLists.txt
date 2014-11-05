set(NFS_RUNPATH "" CACHE STRING  "Disk area which is shared among cluster nodes and can be used as CWD for LSF/RSH jobs.")
set(RSH_SERVERS "" CACHE STRING  "List of nodes which will be used to test the RSH driver")

set(TEST_SOURCES
    ert_tests/__init__.py
    ert_tests/run_tests.py
    ert_tests/test_ert_import.py
    ert_tests/import_tester.py

    ert_tests/config/__init__.py
    ert_tests/config/test_config.py

    ert_tests/cwrap/__init__.py
    ert_tests/cwrap/test_cwrap.py

    ert_tests/ecl/__init__.py
    ert_tests/ecl/test_deprecation.py
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
    ert_tests/ecl/test_ecl_sum_vector.py
    ert_tests/ecl/test_faults.py
    ert_tests/ecl/test_statoil_faults.py
    ert_tests/ecl/test_fault_blocks.py
    ert_tests/ecl/test_region_definition_statoil.py
    ert_tests/ecl/test_region_definition.py
    ert_tests/ecl/test_npv.py
    ert_tests/ecl/test_indexed_read.py

    ert_tests/enkf/__init__.py
    ert_tests/enkf/test_ert_context.py
    ert_tests/enkf/test_enkf.py
    ert_tests/enkf/test_enkf_fs.py
    ert_tests/enkf/test_enkf_fs_manager1.py
    ert_tests/enkf/test_enkf_fs_manager2.py
    ert_tests/enkf/test_enkf_library.py
    ert_tests/enkf/test_enkf_load_results_manually.py
    ert_tests/enkf/test_analysis_config.py
    ert_tests/enkf/test_analysis_iter_config.py
    ert_tests/enkf/test_state_map.py
    ert_tests/enkf/test_time_map.py
    ert_tests/enkf/test_enkf_obs.py
    ert_tests/enkf/test_run_arg.py
    ert_tests/enkf/plot/test_plot_data.py
    ert_tests/enkf/data/test_gen_data.py
    ert_tests/enkf/data/test_gen_data_config.py
    ert_tests/enkf/data/test_gen_kw.py

    ert_tests/analysis/__init__.py
    ert_tests/analysis/test_options_enum.py
    ert_tests/analysis/test_analysis_module.py

    ert_tests/job_queue/__init__.py
    ert_tests/job_queue/test_ert_script.py
    ert_tests/job_queue/test_function_ert_script.py
    ert_tests/job_queue/test_job_queue.py
    ert_tests/job_queue/test_workflow.py
    ert_tests/job_queue/test_workflow_job.py
    ert_tests/job_queue/test_workflow_joblist.py
    ert_tests/job_queue/test_workflow_runner.py
    ert_tests/job_queue/workflow_common.py

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
    ert_tests/util/test_ui_return.py
    ert_tests/util/test_thread_pool.py
    ert_tests/util/test_work_area.py

    ert_tests/gui/__init__.py
    ert_tests/gui/test_model.py
    ert_tests/gui/test_observable.py

    ert_tests/gui/plot/test_plot_metrics_tracker.py

    ert_tests/gui/ide/__init__.py
    ert_tests/gui/ide/test_bool_argument.py
    ert_tests/gui/ide/test_configuration_line.py
    ert_tests/gui/ide/test_configuration_line_builder.py
    ert_tests/gui/ide/test_configuration_line_definition.py
    ert_tests/gui/ide/test_configuration_line_parser.py
    ert_tests/gui/ide/test_ert_keywords.py
    ert_tests/gui/ide/test_float_argument.py
    ert_tests/gui/ide/test_integer_argument.py
    ert_tests/gui/ide/test_percent_argument.py
    ert_tests/gui/ide/test_path_argument.py
    ert_tests/gui/ide/test_path_argument.py
    ert_tests/gui/ide/test_proper_name_argument.py
    ert_tests/gui/ide/test_proper_name_format_argument.py
    ert_tests/gui/ide/test_proper_name_format_string_argument.py
    ert_tests/gui/ide/test_range_string_argument.py
    ert_tests/gui/ide/test_tokens.py

    ert_tests/gui/ide/wizards/__init__.py
    ert_tests/gui/ide/wizards/test_tree_structure.py

    ert_tests/geometry/__init__.py
    ert_tests/geometry/test_polyline.py
    ert_tests/geometry/test_intersection.py
    ert_tests/geometry/test_convex_hull.py
    ert_tests/geometry/test_point_in_polygon.py
    ert_tests/geometry/test_polygon_slicing.py


    ert_tests/run/__init__.py
    ert_tests/run/test_run.py

    ert_tests/server/test_server.py
    ert_tests/server/test_socket.py

    ert_tests/well/test_ecl_well.py
    ert_tests/well/test_ecl_well2.py

    ecl_isosurf.py
    ens_config_test.py
    grav_test.py
    grid_test0.py
    job_test.py
    petrel_kw.py
    poly_test.py
    troll_test.py
    ctest_run.py
    ctest_import.py
)

if (ERT_BUILD_GUI) 
   LIST(APPEND ${TEST_SOURCES} ert_tests/test_ert_gui_import.py)
endif()

add_python_package("python.tests" "${PYTHON_INSTALL_PREFIX}/test" "${TEST_SOURCES}" False)

# The test data is located in the current source directory; that is
# the reason we set that as the working directory for the test
# run. The module to import should then be in the default python
# module search path (i.e. current directory), whereas the location of
# the actual ert package is given by the first argument to ctest.


add_test( NAME python.tests.ert.import
          WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX}
          COMMAND python test/ctest_import.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} "ert")



if (ERT_BUILD_GUI) 
   add_test( NAME python.tests.ert_gui.import
             WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX}
             COMMAND python test/ctest_import.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} "ert_gui")
endif()


#UTIL

add_test( NAME python.tests.ert.util.stringlist
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.util.test_string_list.StringListTest )
          
add_test( NAME python.tests.ert.util.ui_return
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.util.test_ui_return.UIReturnTest )

add_test( NAME python.tests.ert.util.tvector
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.util.test_vectors.UtilTest )

add_test( NAME python.tests.ert.util.ctime
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.util.test_ctime.CTimeTest)


add_test( NAME python.tests.ert.util.hash
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.util.test_hash.HashTest)

add_test( NAME python.tests.ert.util.work_area
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.util.test_work_area.WorkAreaTest)

#add_test( NAME python.tests.ert.util.latex
#          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.util.test_latex.LatexTest )

add_test( NAME python.tests.ert.util.lookup_table
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.util.test_lookup_table.LookupTableTest )

add_test( NAME python.tests.ert.util.matrix
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.util.test_matrix.MatrixTest )

add_test( NAME python.tests.ert.util.stat
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.util.test_stat.StatTest )


#ENKF
add_test( NAME python.tests.ert.enkf.enkf
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.enkf.test_enkf.EnKFTest)

add_test( NAME python.tests.ert.enkf.enkf_obs
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.enkf.test_enkf_obs.EnKFObsTest)

add_test( NAME python.tests.ert.enkf.enkf_load_results_manually
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.enkf.test_enkf_load_results_manually.LoadResultsManuallyTest)

add_test( NAME python.tests.ert.enkf.enkf_fs
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.enkf.test_enkf_fs.EnKFFSTest)

add_test( NAME python.tests.ert.enkf.ert_log
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.enkf.test_ert_log.ErtLogTest)


add_test( NAME python.tests.ert.enkf.enkf_fs_manager1
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.enkf.test_enkf_fs_manager1.EnKFFSManagerTest1)

add_test( NAME python.tests.ert.enkf.enkf_fs_manager2
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.enkf.test_enkf_fs_manager2.EnKFFSManagerTest2)

add_test( NAME python.tests.ert.enkf.enkf_library
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.enkf.test_enkf_library.EnKFLibraryTest)

add_test( NAME python.tests.ert.enkf.analysis_config
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.enkf.test_analysis_config.AnalysisConfigTest)

add_test( NAME python.tests.ert.enkf.analysis_iter_config
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.enkf.test_analysis_iter_config.AnalysisIterConfigTest)

add_test( NAME python.tests.ert.enkf.state_map
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.enkf.test_state_map.StateMapTest)

add_test( NAME python.tests.ert.enkf.time_map
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.enkf.test_time_map.TimeMapTest)

add_test( NAME python.tests.ert.enkf.ecl_config
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.enkf.test_ecl_config.EclConfigTest)

add_test( NAME python.tests.ert.enkf.plot.plot_block_data
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.enkf.plot.test_plot_data.PlotDataTest)

add_test( NAME python.tests.ert.enkf.data.gen_data
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.enkf.data.test_gen_data.GenDataTest)

add_test( NAME python.tests.ert.enkf.data.gen_data_config
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.enkf.data.test_gen_data_config.GenDataConfigTest)

add_test( NAME python.tests.ert.enkf.data.gen_kw
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.enkf.data.test_gen_kw.GenKwTest)

add_test( NAME python.tests.ert.enkf.ert_test_context
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.enkf.test_ert_context.ErtTestContextTest)

add_test( NAME python.tests.ert.enkf.run_arg
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.enkf.test_run_arg.RunArg)




#GUI
if (ERT_BUILD_GUI) 
   add_test( NAME python.tests.ert_gui.models
             WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.gui.test_model.ModelTest)

   add_test( NAME python.tests.ert_gui.models.observable
             WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.gui.test_observable.ObservableTest)

   add_test( NAME python.tests.ert_gui.plot.plot_metrics_tracker
             WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.gui.plot.test_plot_metrics_tracker.PlotMetricTrackerTest)

   add_test( NAME python.tests.ert_gui.ide.bool_argument
             WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.gui.ide.test_bool_argument.BoolArgumentTest)

   add_test( NAME python.tests.ert_gui.ide.integer_argument
             WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.gui.ide.test_integer_argument.IntegerArgumentTest)

    add_test( NAME python.tests.ert_gui.ide.float_argument
              WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
              COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.gui.ide.test_float_argument.FloatArgumentTest)

    add_test( NAME python.tests.ert_gui.ide.path_argument
              WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
              COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.gui.ide.test_path_argument.PathArgumentTest)

    add_test( NAME python.tests.ert_gui.ide.proper_name_argument
              WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
              COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.gui.ide.test_proper_name_argument.ProperNameArgumentTest)

   add_test( NAME python.tests.ert_gui.ide.proper_name_format_argument
             WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.gui.ide.test_proper_name_format_argument.ProperNameFormatArgumentTest)

   add_test( NAME python.tests.ert_gui.ide.proper_name_format_string_argument
             WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.gui.ide.test_proper_name_format_string_argument.ProperNameFormatStringArgumentTest)

    add_test( NAME python.tests.ert_gui.ide.range_string_argument
              WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
              COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.gui.ide.test_range_string_argument.RangeStringArgumentTest)


    add_test( NAME python.tests.ert_gui.ide.test_configuration_line
              WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
              COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.gui.ide.test_configuration_line.ConfigurationLineTest)

    add_test( NAME python.tests.ert_gui.ide.test_configuration_line_builder
              WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
              COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.gui.ide.test_configuration_line_builder.ConfigurationLineBuilderTest)

    add_test( NAME python.tests.ert_gui.ide.test_configuration_line_definition
              WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
              COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.gui.ide.test_configuration_line_definition.ConfigurationLineDefinitionTest)

    add_test( NAME python.tests.ert_gui.ide.test_configuration_line_parser
              WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
              COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.gui.ide.test_configuration_line_parser.ConfigurationLineParserTest)



    add_test( NAME python.tests.ert_gui.ide.ert_keywords
              WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
              COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.gui.ide.test_ert_keywords.ErtKeywordTest)

    add_test( NAME python.tests.ert_gui.ide.tokens
              WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
              COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.gui.ide.test_tokens.TokenTest)

    add_test( NAME python.tests.ert_gui.ide.wizards.tree_structure
              WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
              COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.gui.ide.wizards.test_tree_structure.TreeStructureTest)

endif()

#ECL


add_test( NAME python.tests.ert.ecl.ecl_default
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_ecl_default.EclDefaultTest )

add_test( NAME python.tests.ert.ecl.ecl_file
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_ecl_file.EclFileTest)

add_test( NAME python.tests.ert.ecl.ecl_queue_LOCAL
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_ecl_submit.LocalSubmitTest  )

if (NOT ${NFS_RUNPATH} STREQUAL "")
   add_test( NAME python.tests.ert.ecl.ecl_queue_LSF
             WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_ecl_submit.LSFSubmitTest ${NFS_RUNPATH} )
   set_property( TEST python.tests.ert.ecl.ecl_queue_LSF     PROPERTY LABELS Python:StatoilData:Slow )

   if (NOT ${RSH_SERVERS} STREQUAL "")
      add_test( NAME python.tests.ert.ecl.ecl_queue_RSH
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_ecl_submit.RSHSubmitTest ${NFS_RUNPATH} ${RSH_SERVERS})
      set_property( TEST python.tests.ert.ecl.ecl_queue_RSH     PROPERTY LABELS Python:StatoilData:Slow )
   endif()
endif()


add_test( NAME python.tests.ert.ecl.fortio
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_fortio.FortIOTest)


add_test( NAME python.tests.ert.ecl.ecl_grdecl
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_grdecl.GRDECLTest )

add_test( NAME python.tests.ert.ecl.ecl_grid
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_grid.GridTest )

add_test( NAME python.tests.ert.ecl.ecl_kw
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_kw.KWTest )

add_test( NAME python.tests.ert.ecl.legacy_ecl
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_legacy_ecl.LegacyEclTest)

add_test( NAME python.tests.ert.ecl.ecl_restart
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_restart.RestartTest)

add_test( NAME python.tests.ert.ecl.ecl_region
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_region.RegionTest)

add_test( NAME python.tests.ert.ecl.ecl_rft
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_rft.RFTTest)

add_test( NAME python.tests.ert.ecl.ecl_rft_cell
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_rft_cell.RFTCellTest)

add_test( NAME python.tests.ert.ecl.ecl_sum
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_sum.SumTest)

add_test( NAME python.tests.ert.ecl.ecl_sum_vector
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_ecl_sum_vector.EclSumVectorTest)

add_test( NAME python.tests.ert.ecl.ecl_sum_time_range
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_ecl_sum.EclSumTest)

add_test( NAME python.tests.ert.ecl.layer
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_layer.LayerTest )

add_test( NAME python.tests.ert.ecl.faults
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_faults.FaultTest )

add_test( NAME python.tests.ert.ecl.fault_blocks
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_fault_blocks.FaultBlockTest )

add_test( NAME python.tests.ert.ecl.region_definition_statoil
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_region_definition_statoil.RegionDefinitionTest )

add_test( NAME python.tests.ert.ecl.region_definition
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_region_definition.RegionDefinitionTest )

add_test( NAME python.tests.ert.ecl.fault_blocks_statoil
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_fault_blocks_statoil.FaultBlockTest )

add_test( NAME python.tests.ert.ecl.ecl_npv
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_npv.NPVTest)

add_test( NAME python.tests.ert.ecl.statoil_faults
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_statoil_faults.StatoilFaultTest )

add_test( NAME python.tests.ert.ecl.ecl_deprecation
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_deprecation.DeprecationTest )

add_test( NAME python.tests.ert.ecl.indexed_read
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.ecl.test_indexed_read.EclIndexedReadTest )

#SCHED
add_test( NAME python.tests.ert.sched.sched
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.sched.test_sched.SchedFileTest)

#JOB QUEUE
add_test( NAME python.tests.ert.job_queue.job_status_type_enum
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.job_queue.test_job_queue.JobQueueTest)

add_test( NAME python.tests.ert.job_queue.workflow_job
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.job_queue.test_workflow_job.WorkflowJobTest)

add_test( NAME python.tests.ert.job_queue.workflow
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.job_queue.test_workflow.WorkflowTest)

add_test( NAME python.tests.ert.job_queue.workflow_joblist
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.job_queue.test_workflow_joblist.WorkflowJoblistTest)

add_test( NAME python.tests.ert.job_queue.ert_script
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.job_queue.test_ert_script.ErtScriptTest)

add_test( NAME python.tests.ert.job_queue.function_ert_script
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.job_queue.test_function_ert_script.FunctionErtScriptTest)

add_test( NAME python.tests.ert.job_queue.workflow_runner
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.job_queue.test_workflow_runner.WorkflowRunnerTest)



#CONFIG
add_test( NAME python.tests.ert.config
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.config.test_config.ConfigTest)


#CWRAP
add_test( NAME python.tests.ert.cwrap
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.cwrap.test_cwrap.CWRapTest)

add_test( NAME python.tests.ert.cwrap.basecclass
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.cwrap.test_basecclass.BaseCClassTest)

add_test( NAME python.tests.ert.cwrap.basecenum
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.cwrap.test_basecenum.BaseCEnumTest)

add_test( NAME python.tests.ert.cwrap.basecvalue
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.cwrap.test_basecvalue.BaseCValueTest)


#TESTRUN
add_test( NAME python.tests.ert.test_run
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.run.test_run.RunTest )

#ANALYSIS
add_test( NAME python.tests.ert.analysis.enums
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.analysis.test_options_enum.AnalysisOptionsEnumTest)

add_test( NAME python.tests.ert.analysis.analysis_module
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.analysis.test_analysis_module.AnalysisModuleTest)



#GEOMETRY

add_test( NAME python.tests.ert.geometry.polyline
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.geometry.test_polyline.PolylineTest)

add_test( NAME python.tests.ert.geometry.intersection
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.geometry.test_intersection.IntersectionTest)

add_test( NAME python.tests.ert.geometry.convex_hull
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.geometry.test_convex_hull.ConvexHullTest)

add_test( NAME python.tests.ert.geometry.point_in_polygon
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.geometry.test_point_in_polygon.PointInPolygonTest)

add_test( NAME python.tests.ert.geometry.polygon_slicing
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.geometry.test_polygon_slicing.PolygonSlicingTest)


#SERVER

#add_test( NAME python.tests.ert.server.socket
#          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.server.test_socket.SocketTest)

#add_test( NAME python.tests.ert.server.server
#          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.server.test_server.ServerTest)


# ECL_WELL
add_test( NAME python.tests.ert.well.ecl_well
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.well.test_ecl_well.EclWellTest)


add_test( NAME python.tests.ert.well.ecl_well2
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ert_tests.well.test_ecl_well2.EclWellTest2)


set_property( TEST python.tests.ert.sched.sched           PROPERTY LABELS Python:StatoilData )
set_property( TEST python.tests.ert.ecl.fortio            PROPERTY LABELS Python:StatoilData )
set_property( TEST python.tests.ert.ecl.ecl_grdecl        PROPERTY LABELS Python:StatoilData )
set_property( TEST python.tests.ert.ecl.ecl_restart       PROPERTY LABELS Python:StatoilData )
set_property( TEST python.tests.ert.ecl.ecl_region        PROPERTY LABELS Python:StatoilData )
set_property( TEST python.tests.ert.ecl.ecl_file          PROPERTY LABELS Python:StatoilData )
set_property( TEST python.tests.ert.ecl.ecl_queue_LOCAL   PROPERTY LABELS Python:StatoilData:Slow )
set_property( TEST python.tests.ert.ecl.ecl_sum           PROPERTY LABELS Python:StatoilData )
set_property( TEST python.tests.ert.ecl.ecl_npv           PROPERTY LABELS Python:StatoilData )
set_property( TEST python.tests.ert.ecl.ecl_kw            PROPERTY LABELS Python:StatoilData )
set_property( TEST python.tests.ert.ecl.ecl_rft           PROPERTY LABELS Python:StatoilData )
set_property( TEST python.tests.ert.ecl.ecl_sum_vector    PROPERTY LABELS Python:StatoilData )
set_property( TEST python.tests.ert.ecl.indexed_read      PROPERTY LABELS Python:StatoilData )
set_property( TEST python.tests.ert.ecl.ecl_grid          PROPERTY LABELS Python:StatoilData:Slow )
set_property( TEST python.tests.ert.ecl.ecl_deprecation   PROPERTY LABELS Python )
#set_property( TEST python.tests.import_local             PROPERTY LABELS Python:StatoilBuild )
set_property( TEST python.tests.ert.well.ecl_well         PROPERTY LABELS Python:StatoilData )
set_property( TEST python.tests.ert.well.ecl_well2        PROPERTY LABELS Python:StatoilData )

set_property( TEST python.tests.ert.test_run              PROPERTY LABELS Python)
set_property( TEST python.tests.ert.import                PROPERTY LABELS Python)
set_property( TEST python.tests.ert.util.stringlist       PROPERTY LABELS Python )
set_property( TEST python.tests.ert.util.tvector          PROPERTY LABELS Python )
set_property( TEST python.tests.ert.ecl.faults            PROPERTY LABELS Python )

set_property( TEST python.tests.ert.ecl.region_definition_statoil PROPERTY LABELS Python:StatoilData )
set_property( TEST python.tests.ert.ecl.fault_blocks_statoil  PROPERTY LABELS Python:StatoilData )
set_property( TEST python.tests.ert.ecl.statoil_faults        PROPERTY LABELS Python:StatoilData )
set_property( TEST python.tests.ert.enkf.enkf                 PROPERTY LABELS Python:StatoilData )
set_property( TEST python.tests.ert.enkf.ecl_config           PROPERTY LABELS Python:StatoilData )
set_property( TEST python.tests.ert.enkf.enkf_fs              PROPERTY LABELS Python:StatoilData )
set_property( TEST python.tests.ert.enkf.enkf_fs_manager1     PROPERTY LABELS Python:StatoilData )
set_property( TEST python.tests.ert.enkf.enkf_fs_manager2     PROPERTY LABELS Python:StatoilData )
set_property( TEST python.tests.ert.enkf.enkf_obs             PROPERTY LABELS Python:StatoilData )
set_property( TEST python.tests.ert.enkf.enkf_load_results_manually     PROPERTY LABELS Python:StatoilData )
set_property( TEST python.tests.ert.enkf.ert_test_context     PROPERTY LABELS Python:StatoilData )
set_property( TEST python.tests.ert.enkf.plot.plot_block_data PROPERTY LABELS Python:StatoilData )
set_property( TEST python.tests.ert.enkf.data.gen_data_config PROPERTY LABELS Python:StatoilData )
set_property( TEST python.tests.ert.enkf.data.gen_data        PROPERTY LABELS Python:StatoilData )
set_property( TEST python.tests.ert.ecl.ecl_sum_time_range    PROPERTY LABELS Python:StatoilData )

if (ERT_BUILD_GUI)
   set_property( TEST python.tests.ert_gui.import         PROPERTY ENVIRONMENT "ERT_SHARE_PATH=${PROJECT_SOURCE_PATH}/share")
   set_property( TEST python.tests.ert_gui.import         PROPERTY LABELS Python)
   set_property( TEST python.tests.ert_gui.models         PROPERTY LABELS Python)
   set_property( TEST python.tests.ert_gui.models.observable PROPERTY LABELS Python)
endif()
