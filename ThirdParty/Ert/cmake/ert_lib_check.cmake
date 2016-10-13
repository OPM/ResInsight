# This file contains various checks which will append to the list
# $ERT_EXTERNAL_UTIL_LIBS which should contain all the external library
# dependencies. Observe that all library dependencies go transitively
# through the ert_util library.

if (ERT_HAVE_LAPACK)
   set( ERT_EXTERNAL_UTIL_LIBS ${ERT_EXTERNAL_UTIL_LIBS} ${LAPACK_LIBRARY} ${BLAS_LIBRARY})
   set( CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${LAPACK_LIBRARY} ${BLAS_LIBRARY})
endif()


if (ERT_WINDOWS)
   find_library( SHLWAPI_LIBRARY NAMES Shlwapi )
   set( ERT_EXTERNAL_UTIL_LIBS ${ERT_EXTERNAL_UTIL_LIBS} Shlwapi )
endif()


if (ERT_HAVE_ZLIB)
   set( ERT_EXTERNAL_UTIL_LIBS ${ERT_EXTERNAL_UTIL_LIBS} ${ZLIB_LIBRARY} )
endif()

if (HAVE_PTHREAD)
   set( ERT_EXTERNAL_UTIL_LIBS ${ERT_EXTERNAL_UTIL_LIBS} ${PTHREAD_LIBRARY} )
endif()

find_library( DL_LIBRARY NAMES dl )
find_path( DLFUNC_HEADER dlfcn.h )
if (DL_LIBRARY AND DLFUNC_HEADER)
    set( ERT_EXTERNAL_UTIL_LIBS ${ERT_EXTERNAL_UTIL_LIBS} ${DL_LIBRARY} )
endif()