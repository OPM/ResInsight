if (BUILD_TESTS)

   # Common test binary used in both the Statoil internal tests and the open tests.
   add_executable( ecl_grid_cell_contains ecl_grid_cell_contains.c )
   target_link_libraries( ecl_grid_cell_contains ecl  )

   include( tests.cmake ) 
   if (STATOIL_TESTDATA_ROOT)
     include (statoil_tests.cmake)
   endif()
endif()

# The ecl_win64 application is not built as a proper test integrated
# into the CTEST system. Should be invoked manually on Windows.
if (ERT_WINDOWS) 
   add_executable( ecl_lfs ecl_lfs.c ) 
   target_link_libraries( ecl_lfs ecl ert_util ) 
endif()

