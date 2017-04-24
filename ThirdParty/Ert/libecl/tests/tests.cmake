add_executable( ecl_nnc_geometry ecl_nnc_geometry.c )
target_link_libraries( ecl_nnc_geometry ecl  )
add_test( ecl_nnc_geometry  ${EXECUTABLE_OUTPUT_PATH}/ecl_nnc_geometry )

add_executable( ecl_alloc_grid_dxv_dyv_dzv ecl_alloc_grid_dxv_dyv_dzv.c )
target_link_libraries( ecl_alloc_grid_dxv_dyv_dzv ecl  )
add_test( ecl_alloc_grid_dxv_dyv_dzv  ${EXECUTABLE_OUTPUT_PATH}/ecl_alloc_grid_dxv_dyv_dzv )

add_executable( ecl_alloc_cpgrid ecl_alloc_cpgrid.c )
target_link_libraries( ecl_alloc_cpgrid ecl  )
add_test( ecl_alloc_cpgrid  ${EXECUTABLE_OUTPUT_PATH}/ecl_alloc_cpgrid )

add_executable( ecl_kw_init ecl_kw_init.c )
target_link_libraries( ecl_kw_init ecl  )
add_test( ecl_kw_init ${EXECUTABLE_OUTPUT_PATH}/ecl_kw_init  )

add_executable( ecl_kw_ix_types ecl_kw_ix_types.c )
target_link_libraries( ecl_kw_ix_types ecl  )
add_test( ecl_kw_ix_types ${EXECUTABLE_OUTPUT_PATH}/ecl_kw_ix_types  )

add_executable( ecl_grid_init_fwrite ecl_grid_init_fwrite.c )
target_link_libraries( ecl_grid_init_fwrite ecl  )
add_test( ecl_grid_init_fwrite ${EXECUTABLE_OUTPUT_PATH}/ecl_grid_init_fwrite  )

add_executable( ecl_init_file ecl_init_file.c )
target_link_libraries( ecl_init_file ecl  )
add_test( ecl_init_file ${EXECUTABLE_OUTPUT_PATH}/ecl_kw_init  )

add_executable( ecl_kw_fread ecl_kw_fread.c )
target_link_libraries( ecl_kw_fread ecl  )
add_test( ecl_kw_fread ${EXECUTABLE_OUTPUT_PATH}/ecl_kw_fread  )

add_executable( ecl_valid_basename ecl_valid_basename.c )
target_link_libraries( ecl_valid_basename ecl  )
add_test( ecl_valid_basename ${EXECUTABLE_OUTPUT_PATH}/ecl_valid_basename)

add_executable( ecl_util_make_date_no_shift ecl_util_make_date_no_shift.c )
target_link_libraries( ecl_util_make_date_no_shift ecl  )
add_test( ecl_util_make_date_no_shift ${EXECUTABLE_OUTPUT_PATH}/ecl_util_make_date_no_shift )

add_executable( ecl_sum_writer ecl_sum_writer.c )
target_link_libraries( ecl_sum_writer ecl  )
add_test( ecl_sum_writer ${EXECUTABLE_OUTPUT_PATH}/ecl_sum_writer )

add_executable( ecl_grid_add_nnc ecl_grid_add_nnc.c )
target_link_libraries( ecl_grid_add_nnc ecl  )
add_test( ecl_grid_add_nnc ${EXECUTABLE_OUTPUT_PATH}/ecl_grid_add_nnc )

add_executable( ecl_grid_create ecl_grid_create.c )
target_link_libraries( ecl_grid_create ecl  )
add_test( ecl_grid_create ${EXECUTABLE_OUTPUT_PATH}/ecl_grid_create )

add_executable( ecl_grid_DEPTHZ ecl_grid_DEPTHZ.c )
target_link_libraries( ecl_grid_DEPTHZ ecl  )
add_test( ecl_grid_DEPTHZ ${EXECUTABLE_OUTPUT_PATH}/ecl_grid_DEPTHZ )

add_executable( ecl_grid_reset_actnum ecl_grid_reset_actnum.c )
target_link_libraries( ecl_grid_reset_actnum ecl  )
add_test( ecl_grid_reset_actnum ${EXECUTABLE_OUTPUT_PATH}/ecl_grid_reset_actnum )

add_executable( ecl_nnc_info_test ecl_nnc_info_test.c )
target_link_libraries( ecl_nnc_info_test ecl  )
add_test (ecl_nnc_info_test ${EXECUTABLE_OUTPUT_PATH}/ecl_nnc_info_test )

add_executable( ecl_nnc_vector ecl_nnc_vector.c )
target_link_libraries( ecl_nnc_vector ecl  )
add_test(ecl_nnc_vector ${EXECUTABLE_OUTPUT_PATH}/ecl_nnc_vector )

add_executable( ecl_kw_grdecl ecl_kw_grdecl.c )
target_link_libraries( ecl_kw_grdecl ecl  )
add_test( ecl_kw_grdecl ${EXECUTABLE_OUTPUT_PATH}/ecl_kw_grdecl )

add_executable( ecl_kw_equal ecl_kw_equal.c )
target_link_libraries( ecl_kw_equal ecl  )
add_test( ecl_kw_equal ${EXECUTABLE_OUTPUT_PATH}/ecl_kw_equal )

add_executable( ecl_kw_cmp_string ecl_kw_cmp_string.c )
target_link_libraries( ecl_kw_cmp_string ecl  )
add_test( ecl_kw_cmp_string ${EXECUTABLE_OUTPUT_PATH}/ecl_kw_cmp_string )

add_executable( ecl_util_month_range ecl_util_month_range.c )
target_link_libraries( ecl_util_month_range ecl  )
add_test( ecl_util_month_range ${EXECUTABLE_OUTPUT_PATH}/ecl_util_month_range  )

if (HAVE_UTIL_ABORT_INTERCEPT)
   add_executable( ecl_grid_corner ecl_grid_corner.c )
   target_link_libraries( ecl_grid_corner ecl  )
   add_test( ecl_grid_corner ${EXECUTABLE_OUTPUT_PATH}/ecl_grid_corner )

   add_executable( ecl_layer ecl_layer.c )
   target_link_libraries( ecl_layer ecl  )
   add_test(ecl_layer ${EXECUTABLE_OUTPUT_PATH}/ecl_layer )
endif()

add_executable( ecl_rft_cell ecl_rft_cell.c )
target_link_libraries( ecl_rft_cell ecl  )
add_test( ecl_rft_cell ${EXECUTABLE_OUTPUT_PATH}/ecl_rft_cell )

add_executable( ecl_grid_copy ecl_grid_copy.c )
target_link_libraries( ecl_grid_copy ecl  )
add_test( ecl_grid_copy ${EXECUTABLE_OUTPUT_PATH}/ecl_grid_copy )

add_executable( ecl_get_num_cpu ecl_get_num_cpu_test.c )
target_link_libraries( ecl_get_num_cpu ecl  )
add_test( ecl_get_num_cpu ${EXECUTABLE_OUTPUT_PATH}/ecl_get_num_cpu 
          ${PROJECT_SOURCE_DIR}/libecl/tests/data/num_cpu1 
          ${PROJECT_SOURCE_DIR}/libecl/tests/data/num_cpu2 
          ${PROJECT_SOURCE_DIR}/libecl/tests/data/num_cpu3
          ${PROJECT_SOURCE_DIR}/libecl/tests/data/num_cpu4 )

add_executable( ecl_fault_block_layer ecl_fault_block_layer.c )
target_link_libraries( ecl_fault_block_layer ecl  )
add_test( ecl_fault_block_layer ${EXECUTABLE_OUTPUT_PATH}/ecl_fault_block_layer ) 

add_executable( ecl_grid_export ecl_grid_export.c )
target_link_libraries( ecl_grid_export ecl )
add_test( ecl_grid_export ${EXECUTABLE_OUTPUT_PATH}/ecl_grid_export  )

add_executable( ecl_rst_file ecl_rst_file.c )
target_link_libraries( ecl_rst_file ecl ert_util )
add_test( ecl_rst_file ${EXECUTABLE_OUTPUT_PATH}/ecl_rst_file  )

add_test( ecl_grid_cell_contains1 ${EXECUTABLE_OUTPUT_PATH}/ecl_grid_cell_contains )
