set( source_files geo_surface.c geo_util.c geo_pointset.c geo_region.c geo_polygon.c)
set( header_files geo_surface.h geo_util.h geo_pointset.h geo_region.h geo_polygon.h)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${libutil_build_path} )
include_directories( ${libutil_src_path} )

add_library( geometry ${LIBRARY_TYPE} ${source_files} )
target_link_libraries( geometry util )


#-----------------------------------------------------------------
if (GEOMETRY_INSTALL_PREFIX)
   install(TARGETS geometry DESTINATION ${CMAKE_INSTALL_PREFIX}/${GEOMETRY_INSTALL_PREFIX}/lib)
   install(FILES ${header_files} DESTINATION ${CMAKE_INSTALL_PREFIX}/${GEOMETRY_INSTALL_PREFIX}/include)
else()
   install(TARGETS geometry DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
   install(FILES ${header_files} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
endif()


