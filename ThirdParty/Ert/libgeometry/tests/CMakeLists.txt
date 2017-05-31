add_executable( geo_util_xlines geo_util_xlines.c )
target_link_libraries( geo_util_xlines ert_geometry  )
add_test( geo_util_xlines ${EXECUTABLE_OUTPUT_PATH}/geo_util_xlines )

add_executable( geo_polygon geo_polygon.c )
target_link_libraries( geo_polygon ert_geometry  )
add_test( geo_polygon ${EXECUTABLE_OUTPUT_PATH}/geo_polygon )

add_executable( geo_polygon_collection geo_polygon_collection.c )
target_link_libraries( geo_polygon_collection ert_geometry  )
add_test( geo_polygon_collection ${EXECUTABLE_OUTPUT_PATH}/geo_polygon_collection )

if (STATOIL_TESTDATA_ROOT)
  add_executable( geo_surface geo_surface.c )
  target_link_libraries( geo_surface ert_geometry  )

  add_test( geo_surface ${EXECUTABLE_OUTPUT_PATH}/geo_surface
                        ${PROJECT_SOURCE_DIR}/test-data/Statoil/Geometry/Surface.irap  
                        ${PROJECT_SOURCE_DIR}/test-data/Statoil/Geometry/Surface_incompatible.irap  )
  set_property( TEST geo_surface  PROPERTY LABELS StatoilData )
endif()



               
