if (STATOIL_TESTDATA_ROOT)

   add_executable( rms_file_test rms_file_test.c )
   target_link_libraries( rms_file_test rms test_util )

   add_test( rms_file_test
             ${EXECUTABLE_OUTPUT_PATH}/rms_file_test
             ${PROJECT_SOURCE_DIR}/test-data/Statoil/config/rms_file_test/rms_file)

   set_property( TEST rms_file_test PROPERTY LABELS StatoilData )

endif()