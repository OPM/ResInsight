add_subdirectory( include/ert/util )

add_subdirectory( src )
if (BUILD_TESTS)
   add_subdirectory( tests )
endif()

