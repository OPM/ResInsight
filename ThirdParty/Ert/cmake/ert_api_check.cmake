# This file contains feature checks which affect the API of the final
# product; i.e. if the test for zlib fails the function
# buffer_fwrite_compressed() will not be available in the final
# installation.
# 
# The results of these tests will be assembled in the
# ert/util/ert_api_config.h header; all the symbols in that header will
# have a ERT_ prefix. The generated header is part of the api and can be
# included by other header files in the ert source.

find_library( BLAS_LIBRARY NAMES blas)
if (BLAS_LIBRARY)
   set(ERT_HAVE_BLAS ON)
endif()

find_library( LAPACK_LIBRARY NAMES lapack)
if (LAPACK_LIBRARY)
   set(ERT_HAVE_LAPACK ON)
else()
   set(ERT_HAVE_LAPACK OFF)
endif()

#-----------------------------------------------------------------

find_library( ZLIB_LIBRARY NAMES z )
find_path( ZLIB_HEADER zlib.h /usr/include )
if (ZLIB_LIBRARY AND ZLIB_HEADER)
   set( ERT_HAVE_ZLIB ON )
else()
   if(NOT DEFINED ZLIB_LIBRARY)
      message(STATUS "ZLib library not found - zlib support will not be included." )       
   endif()
   if(NOT DEFINED ZLIB_HEADER)
      message(STATUS "zlib.h not found - zlib support will not be included.")
   endif()
endif()
#-----------------------------------------------------------------

try_compile( ERT_HAVE_ISFINITE ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake/Tests/test_isfinite.c )
find_path( ERT_HAVE_GETOPT getopt.h /usr/include )
find_path( ERT_HAVE_UNISTD unistd.h /usr/include )

check_function_exists( posix_spawn ERT_HAVE_SPAWN )
check_function_exists( opendir ERT_HAVE_OPENDIR )
check_function_exists( symlink ERT_HAVE_SYMLINK )
check_function_exists( readlinkat ERT_HAVE_READLINKAT )
check_function_exists( glob ERT_HAVE_GLOB )
check_function_exists( getuid ERT_HAVE_GETUID )
check_function_exists( regexec ERT_HAVE_REGEXP )
check_function_exists( lockf ERT_HAVE_LOCKF )


check_type_size(time_t SIZE_OF_TIME_T)
if (${SIZE_OF_TIME_T} EQUAL 8)
    try_run( RUN_RESULT COMPILE_RESULT ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake/Tests/test_mktime_before1970.c )
    if (${COMPILE_RESULT})
        if (${RUN_RESULT} EQUAL 0)
            set( ERT_TIME_T_64BIT_ACCEPT_PRE1970 ON )
        endif()
    endif()
endif()


if (ERT_WINDOWS)
   if (CMAKE_SIZEOF_VOID_P EQUAL 8)
      set( ERT_WINDOWS_LFS ON )
   endif()
endif()


if (HAVE_PTHREAD)
   set( ERT_HAVE_THREAD_POOL ON )
endif()


find_program(PING_PATH NAMES ping)
if (PING_PATH)
   set( ERT_HAVE_PING ON )
endif()
