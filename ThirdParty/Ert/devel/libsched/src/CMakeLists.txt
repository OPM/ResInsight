set( source_files sched_history.c group_index.c sched_time.c sched_blob.c well_index.c well_history.c group_history.c sched_types.c sched_kw.c sched_file.c  sched_kw_untyped.c sched_kw_gruptree.c sched_kw_tstep.c sched_kw_dates.c sched_kw_wconhist.c sched_kw_wconinjh.c sched_kw_welspecs.c sched_util.c history.c sched_kw_wconprod.c sched_kw_wconinj.c sched_kw_wconinje.c sched_kw_compdat.c sched_kw_include.c gruptree.c)

set( header_files sched_history.h sched_time.h group_index.h sched_blob.h well_index.h group_history.h well_history.h sched_types.h sched_file.h sched_kw.h sched_kw_untyped.h sched_kw_gruptree.h sched_kw_tstep.h sched_kw_dates.h sched_kw_wconhist.h sched_kw_wconinjh.h sched_kw_welspecs.h sched_util.h  history.h sched_kw_wconprod.h sched_kw_wconinj.h  sched_kw_wconinje.h sched_kw_compdat.h sched_kw_include.h sched_macros.h gruptree.h)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${libutil_build_path} )
include_directories( ${libutil_src_path} )
include_directories( ${libecl_src_path} )

add_library( sched_static  STATIC ${source_files} )
add_library( sched_shared  SHARED ${source_files} )

set_target_properties( sched_static PROPERTIES OUTPUT_NAME sched )
set_target_properties( sched_shared PROPERTIES OUTPUT_NAME sched )

target_link_libraries( sched_static )
target_link_libraries( sched_shared )

#-----------------------------------------------------------------
if (SCHED_INSTALL_PREFIX)
   install(TARGETS sched_static sched_shared DESTINATION ${INSTALL_ROOT}/${SCHED_INSTALL_PREFIX}/lib)
   install(FILES ${header_files} DESTINATION ${INSTALL_ROOT}/${SCHED_INSTALL_PREFIX}/include)
else()
   install(TARGETS sched_static sched_shared DESTINATION ${INSTALL_ROOT}/lib)
   install(FILES ${header_files} DESTINATION ${INSTALL_ROOT}/include)
endif()        


