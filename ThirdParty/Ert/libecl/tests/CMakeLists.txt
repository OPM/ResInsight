if (BUILD_TESTS)
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

