if (USE_LSF)
  set( LSF_INCLUDE_PATH $ENV{LSF_INCLUDE_PATH}      CACHE FILEPATH "Path to LSF header files")
  set( LSF_LIB_PATH     $ENV{LSF_HOME}/lib          CACHE FILEPATH "Path to LSF library files")

  set( LSF_TEST_INCLUDE_PATH ${LSF_INCLUDE_PATH}/lsf )
  find_path( LSF_HEADER lsf.h ${LSF_TEST_INCLUDE_PATH})
  find_library( LSF_LIBRARY NAMES lsf PATHS ${LSF_LIB_PATH}) 

  if (NOT DEFINED LSF_HEADER)
    set( USE_LSF OFF)
  endif()

  if (NOT DEFINED LSF_LIBRARY)
    set( USE_LSF OFF )
  endif()
endif()

if (USE_LSF)
  set( HAVE_LSF ON )
  add_definitions( -DHAVE_LSF )
endif()

include_directories( ${LSF_INCLUDE_PATH} )

add_subdirectory( src )
if (BUILD_APPLICATONS)
   add_subdirectory( applications )
endif()

if (BUILD_TESTS)
   add_subdirectory( tests )
endif()


