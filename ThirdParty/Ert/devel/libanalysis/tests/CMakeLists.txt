add_test( analysis_module_test_RML ${EXECUTABLE_OUTPUT_PATH}/ert_module_test ${LIBRARY_OUTPUT_PATH}/rml_enkf.so )
add_test( analysis_module_test_RMLI ${EXECUTABLE_OUTPUT_PATH}/ert_module_test ${LIBRARY_OUTPUT_PATH}/rmli_enkf.so )


add_executable(analysis_test_external_module analysis_test_external_module.c )
target_link_libraries( analysis_test_external_module analysis util test_util )

add_test( analysis_module_rml  ${EXECUTABLE_OUTPUT_PATH}/analysis_test_external_module "RML_ENKF" ${LIBRARY_OUTPUT_PATH}/rml_enkf.so 41 ITER)
add_test( analysis_module_rmli ${EXECUTABLE_OUTPUT_PATH}/analysis_test_external_module "RMLI_ENKF" ${LIBRARY_OUTPUT_PATH}/rmli_enkf.so 41 ITER)