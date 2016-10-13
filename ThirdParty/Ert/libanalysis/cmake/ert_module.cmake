function( ert_module target args source_files )

    set( build_file ${target}.so )
    set( depends analysis )
    set( arg_string "${target} ${args}")
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

    get_filename_component( module ${target} NAME )
    add_custom_target( ${module} ALL DEPENDS ${build_file} )

endfunction()
