add_executable(analysis_rml_enkf_common analysis_rml_enkf_common.c ../rml_enkf_common.c)
target_link_libraries( analysis_rml_enkf_common analysis util test_util )

add_test( analysis_rml_enkf_common  ${EXECUTABLE_OUTPUT_PATH}/analysis_rml_enkf_common )

