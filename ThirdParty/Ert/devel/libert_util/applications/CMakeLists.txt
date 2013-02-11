link_directories( ${ERT_BINARY_DIR}/libert_util/src )

add_executable( matrix_test matrix_test.c )
target_link_libraries( matrix_test ert_util )
