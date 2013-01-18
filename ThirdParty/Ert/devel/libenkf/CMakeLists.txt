set( ENKF_INSTALL_PREFIX "" CACHE STRING "Prefix for installation of libenkf")

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src/plot )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src )
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/src )
include_directories( ${libecl_src_path} )
include_directories( ${librms_src_path} )
include_directories( ${libconfig_src_path} )
include_directories( ${libjob_queue_src_path} )
include_directories( ${libplot_src_path} )
include_directories( ${libanalysis_src_path} )
include_directories( ${libsched_src_path} )
include_directories( ${libgeometry_src_path} )

link_directories( ${CMAKE_CURRENT_BINARY_DIR}/src )

add_subdirectory( src )
add_subdirectory( applications/ert_tui )
