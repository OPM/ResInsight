link_directories( ${ERT_BINARY_DIR}/libutil/src )

include_directories( ${libutil_build_path} )
include_directories( ${libutil_src_path} )

add_executable( matrix_test matrix_test.c )
target_link_libraries( matrix_test util )
