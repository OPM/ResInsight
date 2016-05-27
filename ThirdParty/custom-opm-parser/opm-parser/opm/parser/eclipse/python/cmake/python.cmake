if (NOT PYTHONINTERP_FOUND)
  find_package (PythonInterp REQUIRED)
endif ()


function(add_python_package target package_path source_files install_package)

  set(build_files "")                       

  foreach (file ${source_files} )
     set( source_file ${CMAKE_CURRENT_SOURCE_DIR}/${file} )
     set( build_file ${PROJECT_BINARY_DIR}/${package_path}/${file} )
     set( install_file ${CMAKE_INSTALL_PREFIX}/${package_path}/${file} )

     add_custom_command(
        OUTPUT  ${build_file}
        COMMAND ${PYTHON_EXECUTABLE}
        ARGS    ${PROJECT_SOURCE_DIR}/opm/parser/eclipse/python/cmake/cmake_pyc ${source_file} ${build_file}
        DEPENDS ${source_file} )
    
     list(APPEND build_files ${build_file} )

     if (install_package)
        install(FILES ${build_file} DESTINATION ${CMAKE_INSTALL_PREFIX}/${package_path})
        install(CODE "execute_process(COMMAND ${PROJECT_SOURCE_DIR}/cmake/cmake_pyc_file ${install_file})")
     endif()
     
  endforeach()
  add_custom_target( ${target} ALL DEPENDS ${build_files})

endfunction()


#-----------------------------------------------------------------


function (addPythonTest TEST_NAME TEST_CLASS)
    set(oneValueArgs LABELS)
    set(multiValueArgs ARGUMENTS ENVIRONMENT)
    cmake_parse_arguments(TEST_OPTIONS "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_test(NAME python.tests.${TEST_NAME}
             WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX}
             COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/opm/parser/eclipse/python/cmake/ctest_run_python ${PROJECT_BINARY_DIR}/${PYTHON_INSTALL_PREFIX} ${TEST_CLASS} ${TEST_OPTIONS_ARGUMENTS})

    if(TEST_OPTIONS_LABELS)
        set_property(TEST python.tests.${TEST_NAME} PROPERTY LABELS "Python:${TEST_OPTIONS_LABELS}")
    else()
        set_property(TEST python.tests.${TEST_NAME} PROPERTY LABELS "Python")
    endif()

    if(TEST_OPTIONS_ENVIRONMENT)
        set_property(TEST python.tests.${TEST_NAME} PROPERTY ENVIRONMENT ${TEST_OPTIONS_ENVIRONMENT})
    endif()
    set_property(TEST python.tests.${TEST_NAME} PROPERTY ENVIRONMENT "PYTHONPATH=${ERT_PYTHON_PATH}:${PYTHONPATH}")
endfunction(addPythonTest)

