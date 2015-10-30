include(CheckSymbolExists)

check_function_exists( fseeko HAVE_FSEEKO )
if (HAVE_HFSEEKO)
   add_definitions( -DHAVE_FSEEKO )                       
endif()

check_function_exists( regexec HAVE_REGEXP )
if (HAVE_REGEXP)
  add_definitions( -DHAVE_REGEXP )
endif()

check_function_exists( realpath HAVE_REALPATH )
if (HAVE_REALPATH)
  add_definitions( -DHAVE_REALPATH )
endif()

check_function_exists( chmod HAVE_CHMOD )
check_type_size( mode_t SIZE_MODE_T )
if (HAVE_CHMOD AND HAVE_SIZE_MODE_T)
  add_definitions( -DHAVE_CHMOD_AND_MODE_T )
endif()

check_function_exists( fork HAVE_FORK )
if (HAVE_FORK)
  add_definitions( -DHAVE_FORK )
endif()

check_function_exists( round HAVE_ROUND )
if (HAVE_ROUND)
  add_definitions( -DHAVE_ROUND )
endif()

check_function_exists( ftruncate HAVE_FTRUNCATE )
if (HAVE_FTRUNCATE)
  add_definitions( -DHAVE_FTRUNCATE )
endif()

check_function_exists( readlinkat HAVE_READLINKAT )
if (HAVE_READLINKAT)
   add_definitions( -DHAVE_READLINKAT )
endif()

check_function_exists( symlink HAVE_SYMLINK )
if (HAVE_SYMLINK)
  add_definitions( -DHAVE_SYMLINK )
endif()

check_function_exists( getuid HAVE_GETUID )
if (HAVE_GETUID)
  add_definitions( -DHAVE_GETUID )
endif()

check_function_exists( _chdir HAVE_WINDOWS_CHDIR)
if (HAVE_WINDOWS_CHDIR)
   add_definitions( -DHAVE_WINDOWS_CHDIR)
else()
   check_function_exists( chdir HAVE_CHDIR)
   if (HAVE_CHDIR)
      add_definitions( -DHAVE_CHDIR)
   else()
      message(FATAL_ERROR "Could not find chdir() / _chdir() functions")
   endif()
endif()

check_function_exists( localtime_r HAVE_LOCALTIME_R )
if (HAVE_LOCALTIME_R)
  add_definitions( -DHAVE_LOCALTIME_R )
endif()

check_function_exists( lockf HAVE_LOCKF )
if (HAVE_LOCKF)
  add_definitions( -DHAVE_LOCKF )
endif()


check_function_exists( glob HAVE_GLOB )
if (HAVE_GLOB)
  add_definitions( -DHAVE_GLOB )
endif()

check_function_exists( fnmatch HAVE_FNMATCH )
if (HAVE_FNMATCH)
  add_definitions( -DHAVE_FNMATCH )
endif()

check_function_exists( fsync HAVE_FSYNC )
if (HAVE_FSYNC)
  add_definitions( -DHAVE_FSYNC )
endif()

check_function_exists( setenv HAVE_SETENV )
if (HAVE_SETENV)
  add_definitions( -DPOSIX_SETENV )
endif()

check_function_exists( opendir HAVE_OPENDIR )
if (HAVE_OPENDIR)
  add_definitions( -DHAVE_OPENDIR )
endif()

check_function_exists( getpwuid HAVE_GETPWUID )
if (HAVE_GETPWUID)
  add_definitions( -DHAVE_GETPWUID )
endif()

# The usleep() check uses the symbol HAVE__USLEEP with double
# underscore to avoid conflict with plplot which defines the
# HAVE_USLEEP symbol.
check_function_exists( usleep HAVE__USLEEP )
if (HAVE__USLEEP)
  add_definitions( -DHAVE__USLEEP )
endif()

check_symbol_exists(_tzname time.h HAVE_WINDOWS_TZNAME)
if (HAVE_WINDOWS_TZNAME)
   add_definitions(-DHAVE_WINDOWS_TZNAME)
else()
   check_symbol_exists(tzname time.h HAVE_TZNAME)
   if (HAVE_TZNAME)
      add_definitions(-DHAVE_TZNAME)
   else()
      message(FATAL_ERROR "Could not find tzname global variable")
   endif()
endif()

check_function_exists(pthread_yield_np HAVE_YIELD_NP)
if (HAVE_YIELD_NP)
  add_definitions(-DHAVE_YIELD_NP)
endif()

check_function_exists(pthread_yield HAVE_YIELD)
if (HAVE_YIELD)
  add_definitions(-DHAVE_YIELD)
endif()


check_function_exists(pthread_timedjoin_np HAVE_TIMEDJOIN)
if (HAVE_TIMEDJOIN)
  add_definitions(-DHAVE_TIMEDJOIN)
endif()

# Checking based on compiling. Some of the code generates warnings, so we just cut down to bare-bone compiler flags.

set( CMAKE_C_FLAGS_main ${CMAKE_C_FLAGS} )
set( CMAKE_CXX_FLAGS_main ${CMAKE_CXX_FLAGS} )

if (NOT ERT_WINDOWS)
  set( CMAKE_C_FLAGS "-std=gnu99" )
  set( CMAKE_CXX_FLAGS "-std=c++0x")
endif()

try_compile( HAVE_ISFINITE ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake/Tests/test_isfinite.c )
if (HAVE_ISFINITE)
  add_definitions( -DHAVE_ISFINITE )
endif()

try_compile( MKDIR_POSIX ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake/Tests/test_mkdir.c )
if (MKDIR_POSIX)
  add_definitions( -DMKDIR_POSIX )
endif()

try_compile( HAVE_PID_T ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake/Tests/test_pid_t.c )
if (HAVE_PID_T)
  add_definitions( -DHAVE_PID_T )
endif()

try_compile( HAVE_VA_COPY ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake/Tests/test_va_copy.c )
if (HAVE_VA_COPY)
   add_definitions( -DHAVE_VA_COPY )
endif()


try_compile( ISREG_POSIX ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake/Tests/test_isreg.c )
if (ISREG_POSIX)
  add_definitions( -DHAVE_ISREG )
endif()

try_compile( HAVE_SIGBUS ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake/Tests/test_have_sigbus.c )
if (HAVE_SIGBUS)
  add_definitions( -DHAVE_SIGBUS )
endif()

try_compile( HAVE_CXX_SHARED_PTR ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake/Tests/test_shared_ptr.cpp )

check_type_size(time_t SIZE_OF_TIME_T)
if (${SIZE_OF_TIME_T} EQUAL 8)
    try_run( RUN_RESULT COMPILE_RESULT ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake/Tests/test_mktime_before1970.c )
    if (${COMPILE_RESULT})
        if (${RUN_RESULT} EQUAL 0)
            add_definitions( -DTIME_T_64BIT_ACCEPT_PRE1970 )
        endif()
    endif()
endif()

set( CMAKE_C_FLAGS ${CMAKE_C_FLAGS_main} )
set( CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS_main} )

