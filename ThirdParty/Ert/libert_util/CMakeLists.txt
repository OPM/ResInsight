add_subdirectory( include/ert/util )

add_subdirectory( src )
if (BUILD_TESTS)
   add_subdirectory( tests )
endif()

if (BUILD_APPLICATIONS)
   add_subdirectory( applications )
endif()
