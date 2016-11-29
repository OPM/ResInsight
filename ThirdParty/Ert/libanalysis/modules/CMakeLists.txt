include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set( RML_SOURCE_FILES
     rml_enkf_config.c
     rml_enkf_log.c
     rml_enkf.c
     rml_enkf_common.c )

set( header_files analysis_module.h enkf_linalg.h analysis_table.h std_enkf.h rml_enkf_common.h)
add_library( rml_enkf  SHARED ${RML_SOURCE_FILES} )
add_library( std_enkf_debug SHARED std_enkf_debug.c )

set_target_properties( rml_enkf PROPERTIES VERSION 1.0 SOVERSION 1.0 PREFIX "")
set_target_properties( std_enkf_debug PROPERTIES VERSION 1.0 SOVERSION 1.0 PREFIX "")

target_link_libraries( rml_enkf analysis dl )
target_link_libraries( std_enkf_debug analysis dl )

if (USE_RUNPATH)
   add_runpath( rml_enkf )
   add_runpath( std_enkf_debug )
endif()

if (BUILD_TESTS)
   add_subdirectory( tests )
endif()

if (INSTALL_ERT)
   install(TARGETS rml_enkf DESTINATION ${CMAKE_INSTALL_LIBDIR})
   install(TARGETS std_enkf_debug DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()


#-----------------------------------------------------------------

# Alternative script based build:
#if (BUILD_TESTS)
#   if (BUILD_APPLICATIONS)
#set( args "--silent --exclude-ert -I${PROJECT_SOURCE_DIR}/libanalysis/include -I${PROJECT_SOURCE_DIR}/libert_util/include -I${CMAKE_CURRENT_SOURCE_DIR} -I${PROJECT_BINARY_DIR}/libert_util/include")


#set( RML_SOURCE_FILES
#     rml_enkf.c
#     rml_enkf_common.c )

#ert_module( ${LIBRARY_OUTPUT_PATH}/rml_enkf ${args} "${RML_SOURCE_FILES}")



