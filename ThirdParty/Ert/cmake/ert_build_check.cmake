# This file contains checks which are used to implement portable
# utility functions. The results of these check are assembled in the
# generated header "ert/util/build_config.h" - that header is NOT part
# of the public api and it should only be included from source files
# as part of the compilation.
#
# Check which affect the final api are implemented in the
# ert_api_check.cmake file.

find_library( PTHREAD_LIBRARY NAMES pthread )
if (PTHREAD_LIBRARY)
   set( HAVE_PTHREAD ON )
   set (CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} pthread)
endif()

if (UNIX)
   set( CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} m )
   set( ERT_EXTERNAL_UTIL_LIBS ${ERT_EXTERNAL_UTIL_LIBS} m )
endif(UNIX)


check_function_exists( localtime_r HAVE_LOCALTIME_R )
check_function_exists( gmtime_r HAVE_GMTIME_R )
check_function_exists( realpath HAVE_REALPATH )
check_function_exists( usleep HAVE__USLEEP )
check_function_exists( fnmatch HAVE_FNMATCH )
check_function_exists( ftruncate HAVE_FTRUNCATE )
check_function_exists( round HAVE_ROUND )
check_function_exists( GetTempPath HAVE_WINDOWS_GET_TEMP_PATH )
check_function_exists( fork HAVE_FORK )
check_function_exists( getpwuid HAVE_GETPWUID )
check_function_exists( fsync HAVE_FSYNC )
check_function_exists( setenv HAVE_POSIX_SETENV )
check_function_exists( chmod HAVE_CHMOD )
check_function_exists( pthread_timedjoin_np HAVE_TIMEDJOIN)
check_function_exists( pthread_yield_np HAVE_YIELD_NP)
check_function_exists( pthread_yield HAVE_YIELD)
check_function_exists( fseeko HAVE_FSEEKO )
check_function_exists( timegm HAVE_TIMEGM )

check_function_exists( _mkdir HAVE_WINDOWS_MKDIR)
if (NOT HAVE_WINDOWS_MKDIR)
   check_function_exists( mkdir HAVE_POSIX_MKDIR)
endif()


check_function_exists( _chdir HAVE_WINDOWS_CHDIR)
if (NOT HAVE_WINDOWS_CHDIR)
   check_function_exists( chdir HAVE_POSIX_CHDIR)
endif()

check_function_exists( _getcwd HAVE_WINDOWS_GETCWD)
if (NOT HAVE_WINDOWS_GETCWD)
   check_function_exists( getcwd HAVE_POSIX_GETCWD)
endif()


include(CheckSymbolExists)
check_symbol_exists(_tzname time.h HAVE_WINDOWS_TZNAME)
check_symbol_exists( tzname time.h HAVE_TZNAME)

find_path( HAVE_EXECINFO execinfo.h /usr/include )

try_compile( HAVE_VA_COPY ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake/Tests/test_va_copy.c )
try_compile( HAVE_SIGBUS ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake/Tests/test_have_sigbus.c )
try_compile( HAVE_PID_T ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake/Tests/test_pid_t.c )
try_compile( HAVE_MODE_T ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake/Tests/test_mode_t.c )


set( BUILD_CXX ON )
try_compile( HAVE_CXX_SHARED_PTR ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake/Tests/test_shared_ptr.cpp )
if (NOT HAVE_CXX_SHARED_PTR)
   set( BUILD_CXX OFF )
endif()

if (HAVE_FORK AND HAVE_PTHREAD AND HAVE_EXECINFO AND HAVE_GETPWUID)
   set( HAVE_UTIL_ABORT_INTERCEPT ON)
   set( HAVE_BACKTRACE ON)
endif()

