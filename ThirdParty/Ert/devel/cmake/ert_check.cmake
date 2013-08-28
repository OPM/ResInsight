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

# The usleep() check uses the symbol HAVE__USLEEP with double
# underscore to avoid conflict with plplot which defines the
# HAVE_USLEEP symbol.
check_function_exists( usleep HAVE__USLEEP )
if (HAVE_OPENDIR)
  add_definitions( -DHAVE__USLEEP )
endif()

# Checking based on compiling. Some of the code generates warnings, so we just cut down to bare-bone compiler flags.

set( CMAKE_C_FLAGS_main ${CMAKE_C_FLAGS} )
set( CMAKE_CXX_FLAGS_main ${CMAKE_CXX_FLAGS} )

if (NOT ERT_WINDOWS)
  set( CMAKE_C_FLAGS "-std=gnu99" )
  set( CMAKE_CXX_FLAGS "")
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

set( CMAKE_C_FLAGS ${CMAKE_C_FLAGS_main} )
set( CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS_main} )
