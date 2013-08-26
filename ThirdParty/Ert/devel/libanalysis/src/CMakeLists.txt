# Common libanalysis library
set( source_files analysis_module.c enkf_linalg.c std_enkf.c sqrt_enkf.c cv_enkf.c bootstrap_enkf.c null_enkf.c fwd_step_enkf.c )
set( header_files analysis_module.h enkf_linalg.h analysis_table.h)
add_library( analysis  SHARED ${source_files} )
set_target_properties( analysis PROPERTIES COMPILE_DEFINITIONS INTERNAL_LINK)
set_target_properties( analysis PROPERTIES VERSION 1.0 SOVERSION 1.0 )

target_link_libraries( analysis ert_util )
if (NEED_LIBDL)
   target_link_libraries( analysis dl )
endif()

if (USE_RUNPATH)
   add_runpath( analysis )
endif()

# List of modules
set( CMAKE_SHARED_MODULE_PREFIX "" )
add_library( std_enkf  MODULE  std_enkf.c )
add_library( sqrt_enkf MODULE sqrt_enkf.c )
add_library( rml_enkf  MODULE rml_enkf.c rml_enkf_common.c )

#-----------------------------------------------------------------

if (INSTALL_ERT)
   install(TARGETS analysis DESTINATION ${CMAKE_INSTALL_LIBDIR})
   install(TARGETS rml_enkf DESTINATION ${CMAKE_INSTALL_LIBDIR})
   foreach(header ${header_files})
       install(FILES ../include/ert/analysis/${header} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ert/analysis)
   endforeach()
endif()

