set( source_files
     EclKW.cpp
     FortIO.cpp )

set( header_files
     EclKW.hpp
     FortIO.hpp )


add_library( eclxx ${LIBRARY_TYPE} ${source_files} )
set_target_properties( eclxx PROPERTIES VERSION 1.0 SOVERSION 1.0 )
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
