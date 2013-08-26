add_executable( geo_surface geo_surface.c )
target_link_libraries( geo_surface ert_geometry )

add_test( geo_surface ${EXECUTABLE_OUTPUT_PATH}/geo_surface
                      ${PROJECT_SOURCE_DIR}/test-data/Statoil/Geometry/Surface.irap  
                      ${PROJECT_SOURCE_DIR}/test-data/Statoil/Geometry/Surface_incompatible.irap  )

set_property( TEST geo_surface  PROPERTY LABELS StatoilData )
               
