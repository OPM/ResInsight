set( source_files config.c conf.c conf_util.c conf_data.c)
set( header_files config.h conf.h conf_data.h)

add_library( config ${LIBRARY_TYPE} ${source_files} )
set_target_properties( config PROPERTIES VERSION 1.0 SOVERSION 1.0 )

target_link_libraries( config ert_util )

install(TARGETS config DESTINATION ${CMAKE_INSTALL_LIBDIR})
foreach(header ${header_files})
    install(FILES ../include/ert/config/${header} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ert/config)
endforeach()
