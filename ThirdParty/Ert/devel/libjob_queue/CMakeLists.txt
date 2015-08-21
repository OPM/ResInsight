set( HAVE_LSF_LIBRARY OFF )

set( ERT_LSF_LIB_PATH     "" CACHE FILEPATH "Path to search for the LSF libraries" )
set( ERT_LSF_INCLUDE_PATH "" CACHE FILEPATH "Path to search for the LSF header files" )

find_path( LSF_HEADER_PATH lsf/lsf.h 
           PATHS ${ERT_LSF_INCLUDE_PATH}) 

find_library( LSF_LIBRARY NAMES lsf PATHS ${ERT_LSF_LIB_PATH}) 



if (LSF_HEADER_PATH)
   if (LSF_LIBRARY)
      set( HAVE_LSF_LIBRARY ON )
   endif()
endif()


if (HAVE_LSF_LIBRARY) 
   include_directories( ${LSF_HEADER_PATH} )     
   add_definitions( -DHAVE_LSF_LIBRARY )
   message(STATUS "Found LSF as ${LSF_HEADER_PATH} and ${LSF_LIBRARY}")
else()
   message(STATUS "LSF not found")     
endif()

add_subdirectory( src )
if (BUILD_APPLICATIONS)
   add_subdirectory( applications )
endif()

if (BUILD_TESTS)
   add_subdirectory( tests )
endif()


