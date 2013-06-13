add_python_package("Python test" "${PYTHON_INSTALL_PREFIX}/test" "ecl_isosurf.py;enkf_test.py;ens_config_test.py;file_test.py;fortio_test.py;grav_test.py;grdecl_test.py;grid_test0.py;grid_test.py;import_test.py;job_test.py;kw_test.py;large_mem_test.py;latex_test.py;petrel_kw.py;poly_test.py;region_test.py;restart_test.py;rft_test.py;sched_test.py;stringlist_test.py;sum_test.py;test_all.py;test_fast.py;test_util.py;troll_test.py;util_test.py;ctest_run.py" True)

# The test date is located in the current source directory; that is
# the reason we set that as the working directory for the test
# run. The module to import should then be in the default python
# module search path (i.e. current directory), whereas the location of
# the actual ert package is given by the first argument to ctest.

add_test( NAME python.import_all
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} import_test )

add_test( NAME python.import_local
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} import_local )

add_test( NAME python.ert.util.stringlist   
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} stringlist_test )

add_test( NAME python.ert.util.tvector      
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} tvector_test )

add_test( NAME python.ert.ecl.ecl_grid      
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} grid_test )

add_test( NAME python.ert.ecl.ecl_kw        
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} kw_test )

add_test( NAME python.ert.ecl.ecl_sum       
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} sum_test )

add_test( NAME python.ert.ecl.ecl_file                                              
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} file_test False)

add_test( NAME python.ert.ecl.ecl_file_slow 
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} file_test True)

add_test( NAME python.ert.ecl.ecl_rft 
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} rft_test)

add_test( NAME python.ert.ecl.ecl_rft_cell 
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
          COMMAND ctest_run.py ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} rft_cell_test)




set_property( TEST python.ert.ecl.ecl_file       PROPERTY LABELS Python:StatoilData )
set_property( TEST python.ert.ecl.ecl_file_slow  PROPERTY LABELS Python:StatoilData:Slow )
set_property( TEST python.ert.ecl.ecl_sum        PROPERTY LABELS Python:StatoilData )
set_property( TEST python.ert.ecl.ecl_kw         PROPERTY LABELS Python:StatoilData )
set_property( TEST python.ert.ecl.ecl_rft        PROPERTY LABELS Python:StatoilData )
set_property( TEST python.ert.ecl.ecl_grid       PROPERTY LABELS Python:StatoilData:Slow )
set_property( TEST python.import_local           PROPERTY LABELS Python:StatoilBuild )

set_property( TEST python.import_all             PROPERTY LABELS Python)
set_property( TEST python.ert.util.stringlist    PROPERTY LABELS Python )
set_property( TEST python.ert.util.tvector       PROPERTY LABELS Python )
set_property( TEST python.import_all             PROPERTY ENVIRONMENT "ERT_SHARE_PATH=${PROJECT_SOURCE_PATH}/share")

