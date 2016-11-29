add_subdirectory( src )
if (BUILD_APPLICATIONS)
   add_subdirectory( applications )
endif()

if (BUILD_TESTS)
   add_subdirectory( tests )
endif()
