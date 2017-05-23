set( source_files geo_surface.c geo_util.c geo_pointset.c geo_region.c geo_polygon.c geo_polygon_collection.c)
set( header_files geo_surface.h geo_util.h geo_pointset.h geo_region.h geo_polygon.h geo_polygon_collection.h)

add_library( ert_geometry ${source_files} )
set_target_properties( ert_geometry PROPERTIES VERSION ${ERT_VERSION_MAJOR}.${ERT_VERSION_MINOR} SOVERSION ${ERT_VERSION_MAJOR})
target_link_libraries( ert_geometry ert_util )
if (USE_RUNPATH)
   add_runpath( ert_geometry )
endif()   


#-----------------------------------------------------------------
if (INSTALL_ERT)
   install(TARGETS ert_geometry DESTINATION ${CMAKE_INSTALL_LIBDIR})
   foreach(header ${header_files})
       install(FILES ../include/ert/geometry/${header} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ert/geometry)
   endforeach()
endif()


