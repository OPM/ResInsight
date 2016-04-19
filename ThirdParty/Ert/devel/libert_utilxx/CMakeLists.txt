if (BUILD_TESTS)
  add_subdirectory( tests )
endif()

set( header_files
     ert_unique_ptr.hpp
)

if (INSTALL_ERT)
    foreach(header ${header_files})
        install(FILES include/ert/util/${header} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ert/util)
    endforeach()
endif()
