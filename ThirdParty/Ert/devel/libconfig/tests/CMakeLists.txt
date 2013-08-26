add_executable( config_append_test config_append_test.c )
target_link_libraries( config_append_test config test_util )

add_executable( config_node_test config_node_test.c )
target_link_libraries( config_node_test config test_util )

add_executable( config_typeOK config_typeOK.c )
target_link_libraries( config_typeOK config test_util )

add_executable( config_typeFail config_typeFail.c )
target_link_libraries( config_typeFail config test_util )

add_executable( config_path_elm config_path_elm.c )
target_link_libraries( config_path_elm config test_util )

add_executable( config_root_path config_root_path.c )
target_link_libraries( config_root_path config test_util )

add_executable( config_include_test config_include_test.c)
target_link_libraries( config_include_test config test_util )

add_executable( config_content_node config_content_node.c)
target_link_libraries( config_content_node config test_util )

add_executable( config_content_item config_content_item.c)
target_link_libraries( config_content_item config test_util )

add_executable( config_argc config_argc.c)
target_link_libraries( config_argc config test_util )

add_executable( config_error config_error.c)
target_link_libraries( config_error config test_util )
add_test( config_error ${EXECUTABLE_OUTPUT_PATH}/config_error )


add_executable( config_config config_config.c)
target_link_libraries( config_config config test_util )
add_test( config_config        ${EXECUTABLE_OUTPUT_PATH}/config_config )

add_executable( config_schema_item config_schema_item.c)
target_link_libraries( config_schema_item config test_util )
add_test( config_schema_item ${EXECUTABLE_OUTPUT_PATH}/config_schema_item )

add_test( config_typeOK        ${EXECUTABLE_OUTPUT_PATH}/config_typeOK       ${CMAKE_CURRENT_SOURCE_DIR}/data/type_testOK )
add_test( config_typeFail      ${EXECUTABLE_OUTPUT_PATH}/config_typeFail     ${CMAKE_CURRENT_SOURCE_DIR}/data/type_testFail )
add_test( config_append_test   ${EXECUTABLE_OUTPUT_PATH}/config_append_test  ${CMAKE_CURRENT_SOURCE_DIR}/data/append_test )
add_test( config_node_test     ${EXECUTABLE_OUTPUT_PATH}/config_node_test    ${CMAKE_CURRENT_SOURCE_DIR}/data/append_test )
add_test( config_path_elm      ${EXECUTABLE_OUTPUT_PATH}/config_path_elm )
add_test( config_content_node  ${EXECUTABLE_OUTPUT_PATH}/config_content_node )
add_test( config_content_item  ${EXECUTABLE_OUTPUT_PATH}/config_content_item ${CMAKE_CURRENT_SOURCE_DIR}/data/content_item_test)
add_test( config_include_test  ${EXECUTABLE_OUTPUT_PATH}/config_include_test ${CMAKE_CURRENT_SOURCE_DIR}/data include_test )
add_test( config_root_path     ${EXECUTABLE_OUTPUT_PATH}/config_root_path ${CMAKE_CURRENT_SOURCE_DIR}/data )
add_test( config_argc          ${EXECUTABLE_OUTPUT_PATH}/config_argc      ${CMAKE_CURRENT_SOURCE_DIR}/data/argc_OK ${CMAKE_CURRENT_SOURCE_DIR}/data/argc_less ${CMAKE_CURRENT_SOURCE_DIR}/data/argc_more)
