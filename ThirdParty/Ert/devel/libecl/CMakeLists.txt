add_subdirectory( src )
#if (BUILD_APPLICATIONS OR BUILD_ECL_SUMMARY)
#   add_subdirectory( applications )
#endif()

add_subdirectory( applications )
if (BUILD_TESTS)
   add_subdirectory( tests )
endif()

