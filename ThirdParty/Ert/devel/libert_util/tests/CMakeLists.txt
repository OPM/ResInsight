link_directories( ${ERT_BINARY_DIR}/libert_util/src )

add_executable( ert_util_type_vector_test ert_util_type_vector_test.c )
target_link_libraries( ert_util_type_vector_test ert_util )
add_test( ert_util_type_vector_test ${EXECUTABLE_OUTPUT_PATH}/ert_util_type_vector_test )

add_executable( ert_util_vector_test ert_util_vector_test.c )
target_link_libraries( ert_util_vector_test ert_util )
add_test( ert_util_vector_test ${EXECUTABLE_OUTPUT_PATH}/ert_util_vector_test )

add_executable( ert_util_stringlist_test ert_util_stringlist_test.c )
target_link_libraries( ert_util_stringlist_test ert_util )
add_test( ert_util_stringlist_test ${EXECUTABLE_OUTPUT_PATH}/ert_util_stringlist_test )

if (WITH_LATEX)
   add_executable( ert_util_latex_test ert_util_latex_test.c )
   target_link_libraries(ert_util_latex_test ert_util )
   add_test( ert_util_latex_test ${EXECUTABLE_OUTPUT_PATH}/ert_util_latex_test ${CMAKE_CURRENT_SOURCE_DIR}/data/latex_OK.tex )
endif()
