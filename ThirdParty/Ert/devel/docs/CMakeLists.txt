set( ERT_DOC_INSTALL_PATH "" CACHE PATH "Absolute path to install documentation *in addition* to $PREFIX/documentation")

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Static DESTINATION ${PROJECT_BINARY_DIR}/tmp_doc/)
add_custom_target(doc_out ALL 
                          COMMAND ${CMAKE_COMMAND}  -Dccsd=${CMAKE_CURRENT_SOURCE_DIR} -Dpbd=${PROJECT_BINARY_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/build_doc.cmake 
                          DEPENDS enkf )                                  

INSTALL( DIRECTORY ${PROJECT_BINARY_DIR}/tmp_doc/_build/ DESTINATION ${CMAKE_INSTALL_PREFIX}/documentation )
if (ERT_DOC_INSTALL_PATH)
   INSTALL( DIRECTORY ${PROJECT_BINARY_DIR}/tmp_doc/_build/ DESTINATION ${ERT_DOC_INSTALL_PATH} )
endif()
