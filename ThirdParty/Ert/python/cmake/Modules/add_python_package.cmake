function(add_python_package target package_path source_files install_package)

  set(build_files "")                       

  foreach (file ${source_files} )
     string(SUBSTRING ${file} 0 1 first_char)
     string(SUBSTRING ${file} 1 1 second_char)
     if (first_char STREQUAL "/" OR second_char STREQUAL ":")
       set( source_file ${file} )
       set( build_file ${file} )
       file(RELATIVE_PATH file ${CMAKE_CURRENT_BINARY_DIR} ${file})
       set(dependent_target)
     else()
       set( source_file ${CMAKE_CURRENT_SOURCE_DIR}/${file} )
       set( build_file ${PROJECT_BINARY_DIR}/${package_path}/${file} )
       set(dependent_target DEPENDS ${source_file})
     endif()
     if("$ENV{DESTDIR}" STREQUAL "")
       set( install_file ${CMAKE_INSTALL_PREFIX}/${package_path}/${file} )
     else()
       set( install_file $ENV{DESTDIR}/${CMAKE_INSTALL_PREFIX}/${package_path}/${file} )
     endif()

     add_custom_command(
        OUTPUT  ${build_file}
        COMMAND ${PYTHON_EXECUTABLE}
        ARGS    ${PROJECT_BINARY_DIR}/bin/cmake_pyc ${source_file} ${build_file}
        ${dependent_target})
    
     list(APPEND build_files ${build_file} )

     if (install_package)
        install(FILES ${build_file} DESTINATION ${CMAKE_INSTALL_PREFIX}/${package_path})
        install(CODE "execute_process(COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_BINARY_DIR}/bin/cmake_pyc_file ${install_file})")
     endif()
     
  endforeach()
  add_custom_target( ${target} ALL DEPENDS ${build_files})

endfunction()
