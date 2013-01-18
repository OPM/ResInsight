set( source_files well_state.c well_conn.c well_info.c well_ts.c well_branch.c well_path.c )
set( header_files well_state.h well_const.h well_conn.h well_info.h well_ts.h well_branch.h well_path.h )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${libgeometry_src_path} )
include_directories( ${libecl_src_path} )


add_library( well ${LIBRARY_TYPE} ${source_files} )
target_link_libraries( well ecl )

#-----------------------------------------------------------------
if (WELL_INSTALL_PREFIX)
   install(TARGETS well DESTINATION ${CMAKE_INSTALL_PREFIX}/${WELL_INSTALL_PREFIX}/lib)
   install(FILES ${header_files} DESTINATION ${CMAKE_INSTALL_PREFIX}/${WELL_INSTALL_PREFIX}/include)
else()
   install(TARGETS well DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
   install(FILES ${header_files} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
endif()        


