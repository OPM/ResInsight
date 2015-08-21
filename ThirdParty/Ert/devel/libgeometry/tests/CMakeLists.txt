add_executable( geo_surface geo_surface.c )
target_link_libraries( geo_surface ert_geometry test_util )

add_test( geo_surface ${EXECUTABLE_OUTPUT_PATH}/geo_surface
                      ${PROJECT_SOURCE_DIR}/test-data/Statoil/Geometry/Surface.irap  
                      ${PROJECT_SOURCE_DIR}/test-data/Statoil/Geometry/Surface_incompatible.irap  )


add_executable( geo_util_xlines geo_util_xlines.c )
target_link_libraries( geo_util_xlines ert_geometry test_util )
add_test( geo_util_xlines ${EXECUTABLE_OUTPUT_PATH}/geo_util_xlines )

add_executable( geo_polygon geo_polygon.c )
target_link_libraries( geo_polygon ert_geometry test_util )
add_test( geo_polygon ${EXECUTABLE_OUTPUT_PATH}/geo_polygon )

add_executable( geo_polygon_collection geo_polygon_collection.c )
target_link_libraries( geo_polygon_collection ert_geometry test_util )
add_test( geo_polygon_collection ${EXECUTABLE_OUTPUT_PATH}/geo_polygon_collection )

set_property( TEST geo_surface  PROPERTY LABELS StatoilData )
               
