set( SCHED_INSTALL_PREFIX "" CACHE STRING "Prefix for installation of libsched")

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src )
add_subdirectory( src )
add_subdirectory( applications )
