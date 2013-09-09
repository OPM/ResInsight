function( ert_module module args source_files )
    
    set( build_file ${CMAKE_CURRENT_BINARY_DIR}/${module}.so )
    set( depends analysis )
    set( arg_string "${module} ${args}")
    separate_arguments( arg_list UNIX_COMMAND "${arg_string}")
    foreach (src_file ${source_files} )
        list(APPEND arg_list ${CMAKE_CURRENT_SOURCE_DIR}/${src_file} )
        list(APPEND depends ${CMAKE_CURRENT_SOURCE_DIR}/${src_file} )
    endforeach()

    add_custom_command(
       OUTPUT  ${build_file}
       COMMAND ${PROJECT_SOURCE_DIR}/libanalysis/script/ert_module 
       ARGS    ${arg_list}
       DEPENDS ${depends})

    install(FILES ${build_file} DESTINATION ${CMAKE_INSTALL_LIBDIR})
    add_custom_target( ${module} ALL DEPENDS ${build_file} )

endfunction()
