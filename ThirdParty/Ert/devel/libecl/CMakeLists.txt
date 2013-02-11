add_subdirectory( src )
if (BUILD_APPLICATONS OR BUILD_ECL_SUMMARY)
   add_subdirectory( applications )
endif()

if (BUILD_TESTS)
   add_subdirectory( tests )
endif()

