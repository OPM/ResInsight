set( ANALYSIS_INSTALL_PREFIX "" CACHE STRING "Prefix for installation of libanalysis")

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src )
include_directories( ${libutil_build_path} )
include_directories( ${libutil_src_path} )

add_subdirectory( src )
