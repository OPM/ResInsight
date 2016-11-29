ert_module_name( VAR_RML rml_enkf ${LIBRARY_OUTPUT_PATH} )

add_executable(analysis_test_external_module analysis_test_external_module.c )
target_link_libraries( analysis_test_external_module analysis util test_util )

add_test( analysis_module_rml ${EXECUTABLE_OUTPUT_PATH}/analysis_test_external_module "RML_ENKF" ${VAR_RML} 41 
          ITER:45 
          USE_PRIOR:False 
          LAMBDA_REDUCE:0.10 
          LAMBDA_INCREASE:2.5 
          ENKF_TRUNCATION:0.77 
          LAMBDA0:0.25  
          LAMBDA_MIN:0.01
          LOG_FILE:LogFile.txt 
          CLEAR_LOG:True 
          LAMBDA_RECALCULATE:True )


add_executable( analysis_test_module_info analysis_test_module_info.c )
target_link_libraries( analysis_test_module_info analysis util test_util)
add_test( analysis_test_module_info ${EXECUTABLE_OUTPUT_PATH}/analysis_test_module_info )
