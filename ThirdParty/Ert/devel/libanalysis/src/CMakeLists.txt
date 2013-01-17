# Common libanalysis library
set( source_files analysis_module.c enkf_linalg.c std_enkf.c sqrt_enkf.c cv_enkf.c bootstrap_enkf.c null_enkf.c)
set( header_files analysis_module.h enkf_linalg.h analysis_table.h)
add_library( analysis  SHARED ${source_files} )
set_target_properties( analysis PROPERTIES COMPILE_DEFINITIONS INTERNAL_LINK)

target_link_libraries( analysis  )


# List of modules
set( CMAKE_SHARED_MODULE_PREFIX "" )
add_library( std_enkf  MODULE  std_enkf.c )
add_library( sqrt_enkf MODULE sqrt_enkf.c )

#-----------------------------------------------------------------

if (ANALYSIS_INSTALL_PREFIX)
   install(TARGETS analysis DESTINATION ${INSTALL_ROOT}/${ANALYSIS_INSTALL_PREFIX}/lib)
   install(FILES ${header_files} DESTINATION ${INSTALL_ROOT}/${ANALYSIS_INSTALL_PREFIX}/include)
else()
   install(TARGETS analysis DESTINATION ${INSTALL_ROOT}/lib)
   install(FILES ${header_files} DESTINATION ${INSTALL_ROOT}/include)
endif()        


