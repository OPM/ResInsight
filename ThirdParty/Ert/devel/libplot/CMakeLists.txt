set( PLPLOT_ROOT "" CACHE FILEPATH "Root path for PLPLOT installation")
set( PLOT_INSTALL_PREFIX "" CACHE STRING "Prefix for installation of libplot")

set( PLPLOT_INCLUDE_PATH ${PLPLOT_ROOT}/include )
set( PLPLOT_LIB_PATH     ${PLPLOT_ROOT}/lib )

find_library( PLPLOT_LIBRARY NAMES  plplotd  PATHS ${PLPLOT_LIB_PATH} )
find_path( PLPLOT_HEADER  plplot/plplot.h  ${PLPLOT_INCLUDE_PATH} )

include_directories( ${PLPLOT_HEADER} )

add_subdirectory( src )
