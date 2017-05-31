set( source_files
     FortIO.cpp
     Smspec.cpp
     EclFilename.cpp
     )

set( header_files
     EclKW.hpp
     FortIO.hpp
     Smspec.hpp
     EclFilename.hpp
     )


add_library( eclxx ${source_files} )
set_target_properties( eclxx PROPERTIES VERSION ${ERT_VERSION_MAJOR}.${ERT_VERSION_MINOR} SOVERSION ${ERT_VERSION_MAJOR})
if (USE_RUNPATH)
   add_runpath( eclxx )
endif()
target_link_libraries( eclxx ecl )


if (INSTALL_ERT) 
    install(TARGETS eclxx DESTINATION ${CMAKE_INSTALL_LIBDIR})
    foreach(header ${header_files})
        install(FILES ../include/ert/ecl/${header} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ert/ecl)
    endforeach()
endif()
