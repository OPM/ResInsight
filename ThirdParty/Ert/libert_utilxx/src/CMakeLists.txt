set( source_files
     TestArea.cpp
     )

set( header_files
     TestArea.hpp
     ert_unique_ptr.hpp
)


add_library( ert_utilxx ${source_files} )
set_target_properties( ert_utilxx PROPERTIES VERSION ${ERT_VERSION_MAJOR}.${ERT_VERSION_MINOR} SOVERSION ${ERT_VERSION_MAJOR})
if (USE_RUNPATH)
   add_runpath( ert_utilxx )
endif()
target_link_libraries( ert_utilxx ert_util )


if (INSTALL_ERT) 
    install(TARGETS ert_utilxx DESTINATION ${CMAKE_INSTALL_LIBDIR})
    foreach(header ${header_files})
        install(FILES ../include/ert/util/${header} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ert/util)
    endforeach()
endif()
