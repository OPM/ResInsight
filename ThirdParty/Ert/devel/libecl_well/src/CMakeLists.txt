set( source_files well_state.c well_conn.c well_info.c well_ts.c well_branch.c well_path.c )
set( header_files well_state.h well_const.h well_conn.h well_info.h well_ts.h well_branch.h well_path.h )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${libgeometry_src_path} )


add_library( ecl_well ${LIBRARY_TYPE} ${source_files} )
set_target_properties( ecl_well PROPERTIES VERSION 1.0 SOVERSION 1.0 )
target_link_libraries( ecl_well ecl )

#-----------------------------------------------------------------
if (INSTALL_ERT) 
   install(TARGETS ecl_well DESTINATION ${CMAKE_INSTALL_LIBDIR})
   foreach(header ${header_files})
       install(FILES ../include/ert/ecl_well/${header} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ert/ecl_well)
   endforeach()
endif()


