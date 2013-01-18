set( RMS_INSTALL_PREFIX "" CACHE STRING "Prefix for installation of librms")

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src )
include_directories( ${libecl_src_path} )

add_subdirectory( src )
add_subdirectory( applications )
