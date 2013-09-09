set( source_files config.c config_error.c config_schema_item.c config_content_item.c config_content_node.c config_root_path.c config_path_elm.c conf.c conf_util.c conf_data.c)
set( header_files config.h config_error.h config_schema_item.h config_content_item.h config_content_node.h config_root_path.h config_path_elm.h conf.h conf_data.h)

add_library( config ${LIBRARY_TYPE} ${source_files} )
set_target_properties( config PROPERTIES VERSION 1.0 SOVERSION 1.0 )

target_link_libraries( config ert_util )
if (USE_RUNPATH)
   add_runpath( config )
endif()

if (INSTALL_ERT)
   install(TARGETS config DESTINATION ${CMAKE_INSTALL_LIBDIR})
   foreach(header ${header_files})
       install(FILES ../include/ert/config/${header} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ert/config)
   endforeach()
endif()
