set( PLPLOT_ROOT "" CACHE FILEPATH "Root path for PLPLOT installation")
set( PLOT_INSTALL_PREFIX "" CACHE STRING "Prefix for installation of libplot")

if(PLPLOT_ROOT)
  set( PLPLOT_INCLUDE_PATH ${PLPLOT_ROOT}/include )
  set( PLPLOT_LIB_PATH     ${PLPLOT_ROOT}/lib )
else()
  set( PLPLOT_INCLUDE_PATH )
  set( PLPLOT_LIB_PATH )
endif()

list(APPEND CMAKE_LIBRARY_PATH ${PLPLOT_LIB_PATH})
list(APPEND CMAKE_INCLUDE_PATH ${PLPLOT_INCLUDE_PATH})

find_library(PLPLOT_LIBRARY NAMES plplotd)
find_path( PLPLOT_HEADER  plplot/plplot.h)

include_directories( ${PLPLOT_INCLUDE_PATH} )
include_directories( ${PLPLOT_HEADER} )

add_subdirectory( src )

if (BUILD_TESTS)
   add_subdirectory( tests )
endif()

