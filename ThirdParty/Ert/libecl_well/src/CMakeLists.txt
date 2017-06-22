set( source_files
    well_state.c
    well_conn.c
    well_info.c
    well_ts.c
    well_conn_collection.c
    well_segment.c
    well_segment_collection.c
    well_branch_collection.c
    well_rseg_loader.c
)

set( header_files
    well_state.h
    well_const.h
    well_conn.h
    well_info.h
    well_ts.h
    well_conn_collection.h
    well_segment.h
    well_segment_collection.h
    well_branch_collection.h
    well_rseg_loader.h
)

if (NOT ERT_WINDOWS)
   set_property( SOURCE well_branch_collection.c well_segment.c well_segment_collection.c well_conn_collection.c well_conn.c PROPERTY COMPILE_FLAGS "-Werror")
endif()


include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${libgeometry_src_path} )


add_library( ecl_well ${source_files} )
set_target_properties( ecl_well PROPERTIES VERSION ${ERT_VERSION_MAJOR}.${ERT_VERSION_MINOR} SOVERSION ${ERT_VERSION_MAJOR} )
target_link_libraries( ecl_well ecl )
if (USE_RUNPATH)
   add_runpath( ecl_well )
endif()   


#-----------------------------------------------------------------
if (INSTALL_ERT) 
   install(TARGETS ecl_well DESTINATION ${CMAKE_INSTALL_LIBDIR})
   foreach(header ${header_files})
       install(FILES ../include/ert/ecl_well/${header} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ert/ecl_well)
   endforeach()
endif()


