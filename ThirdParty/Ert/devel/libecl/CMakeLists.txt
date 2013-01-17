set( ECL_INSTALL_PREFIX "" CACHE STRING "Prefix for installation of libecl")

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src )
include_directories( ${libutil_build_path} )
include_directories( ${libutil_src_path} )
include_directories( ${libgeometry_src_path} )

add_subdirectory( src )
add_subdirectory( applications )

#if (BUILD_CXX)
#   add_subdirectory( cxx )
#endif()
