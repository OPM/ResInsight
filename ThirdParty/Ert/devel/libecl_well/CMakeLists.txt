add_subdirectory( src )
if (BUILD_APPLICATONS)
   add_subdirectory( applications )
endif()

if (BUILD_TESTS)
   add_subdirectory( tests )
endif()
