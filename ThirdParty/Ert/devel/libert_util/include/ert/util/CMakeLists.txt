set(header_files)

foreach (type int double bool long time_t size_t float)
  set(TYPE ${type} )      
  set(header_target ${type}_vector.h)    

  configure_file( vector_template.h ${header_target})

  list( APPEND header_files ${CMAKE_CURRENT_BINARY_DIR}/${header_target} )
endforeach( type )

if (MSVC)
  configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/msvc_stdbool.h ${CMAKE_CURRENT_BINARY_DIR}/stdbool.h)
  list( APPEND header_files ${CMAKE_CURRENT_BINARY_DIR}/stdbool.h)
endif()

configure_file(build_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/build_config.h)
configure_file(ert_api_config.h.in  ${CMAKE_CURRENT_BINARY_DIR}/ert_api_config.h)
list( APPEND header_files ${CMAKE_CURRENT_BINARY_DIR}/ert_api_config.h)

if (INSTALL_ERT)
   foreach(header ${header_files})
       install(FILES ${header} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ert/util)
   endforeach()
endif()




