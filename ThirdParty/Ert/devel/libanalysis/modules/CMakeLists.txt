set( args "--silent --exclude-ert -I${PROJECT_SOURCE_DIR}/libanalysis/include -I${PROJECT_SOURCE_DIR}/libert_util/include -I${CMAKE_CURRENT_SOURCE_DIR} -I${PROJECT_BINARY_DIR}/libert_util/include")

set( RML_SOURCE_FILES 
     rml_enkf.c 
     rml_enkf_common.c )

set( RMLI_SOURCE_FILES 
     rml_enkf_imodel.c
     rml_enkf_common.c )

ert_module( ${LIBRARY_OUTPUT_PATH}/rml_enkf ${args}  "${RML_SOURCE_FILES}")
ert_module( ${LIBRARY_OUTPUT_PATH}/rmli_enkf ${args} "${RMLI_SOURCE_FILES}")
