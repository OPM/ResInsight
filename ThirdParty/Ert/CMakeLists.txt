cmake_minimum_required( VERSION 2.8.12 ) # If you are creating Python wrappers for Windows, the actual version requirement is 3.4
project( ERT C CXX )

include(GNUInstallDirs)
include(TestBigEndian)

if(POLICY CMP0042)
  cmake_policy(SET CMP0042 OLD)
endif()

#-----------------------------------------------------------------

set( ECL_VERSION_MAJOR 2 )   # Remember to update release notes whenever
set( ECL_VERSION_MINOR 3 )   # you change the ERT_VERSION_MINOR or MAJOR
set( ECL_VERSION_MICRO git ) # with "new in Ert Version X.X.X"!

# If the micro version is not integer, that should be interpreted as a
# development version leading towards version MAJOR.MINOR.0

execute_process(COMMAND date "+%Y-%m-%d %H:%M:%S" OUTPUT_VARIABLE ECL_BUILD_TIME )
string(STRIP "${ECL_BUILD_TIME}" ECL_BUILD_TIME)

find_package(Git)
if(GIT_FOUND)
   execute_process(
     COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
     OUTPUT_VARIABLE GIT_COMMIT
     OUTPUT_STRIP_TRAILING_WHITESPACE
   )

   execute_process(
     COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
     OUTPUT_VARIABLE GIT_COMMIT_SHORT
     OUTPUT_STRIP_TRAILING_WHITESPACE
   )
else()
    set( GIT_COMMIT "unknown (git not found!)")
    set( GIT_COMMIT_SHORT "unknown (git not found!)")
    message( WARNING "Git not found. Build will not contain git revision info." )
endif()

#-----------------------------------------------------------------

option( BUILD_TESTS         "Should the tests be built"                               OFF)
option( BUILD_APPLICATIONS  "Should we build small utility applications"              OFF)
option( BUILD_ECL_SUMMARY   "Build the commandline application ecl_summary"           OFF)
option( BUILD_PYTHON        "Run py_compile on the python wrappers"                   OFF)
option( ENABLE_PYTHON       "Build and install the Python wrappers"                   OFF)
option( BUILD_SHARED_LIBS   "Build shared libraries"                                  ON )
option( ERT_USE_OPENMP      "Use OpenMP"                                              OFF )
option( RST_DOC             "Build RST documentation"                                 OFF)
option( ERT_BUILD_CXX       "Build some CXX wrappers"                                 ON)
option( USE_RPATH           "Don't strip RPATH from libraries and binaries"           OFF)
option( INSTALL_ERT_LEGACY  "Add ert legacy wrappers"                                 OFF)

set(STATOIL_TESTDATA_ROOT "" CACHE PATH  "Root to Statoil internal testdata")
if (EXISTS ${STATOIL_TESTDATA_ROOT})
  set( LINK "${CMAKE_CURRENT_SOURCE_DIR}/test-data/Statoil" )
  if (EXISTS ${LINK})
    EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E remove "${LINK}")
  endif()

  EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E create_symlink "${STATOIL_TESTDATA_ROOT}" "${LINK}")
  message(STATUS "Linking testdata: ${LINK} -> ${STATOIL_TESTDATA_ROOT}")

  set(_statoil_test_data ${CMAKE_SOURCE_DIR}/test-data/Statoil)
  set(_eclpath ${_statoil_test_data}/ECLIPSE)
  set(_geopath ${_statoil_test_data}/Geometry)
endif()


if (BUILD_PYTHON)
  set(ENABLE_PYTHON ON)
  message(WARNING "The option 'BUILD_PYTHON' is deprecated - use ENABLE_PYTHON.")
endif()

# output libs to some lib/ path for testing
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin)

if (USE_RPATH)
    SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif ()

ENABLE_TESTING()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   message(STATUS "Found Linux")
   set(ERT_LINUX TRUE )
   add_definitions( -DERT_LINUX )
   set( ERT_BINARY_POSTFIX .${ERT_VERSION_MAJOR}.${ERT_VERSION_MINOR} )
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   message(STATUS "Found Darwin")
   set(ERT_LINUX TRUE )
   set(ERT_MAC TRUE)
   add_definitions( -DERT_LINUX )
   set( ERT_BINARY_POSTFIX .${ERT_VERSION_MAJOR}.${ERT_VERSION_MINOR} )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
   message(STATUS "Found Windows")
   set(ERT_WINDOWS TRUE)
   add_definitions( -DERT_WINDOWS )
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
   message(STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
   set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
       "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
   # Set the possible values of build type for cmake-gui
   set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
                "MinSizeRel" "RelWithDebInfo")
endif()

# Treat warnings as errors if not on Windows
if (NOT ERT_WINDOWS)
   set( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -std=gnu99 -Wall -Wno-unknown-pragmas ")
   set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unknown-pragmas -Wno-unused-result -Wno-unused-parameter" )
endif()

if (MSVC)
    add_definitions( "/W3 /D_CRT_SECURE_NO_WARNINGS /wd4996" )
endif()


list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)
find_package(CXX11Features)

#-----------------------------------------------------------------

# create targets for (required and optional) dependencies to link to

find_package(OpenMP)
if (NOT OPENMP_FOUND)
    message(STATUS "OpenMP package not found")
    set(ERT_USE_OPENMP OFF)
endif ()

if (NOT ERT_USE_OPENMP)
    message(STATUS "OpenMP disabled")
endif ()

# when cmake can be upgraded to >= 3.0, dependencies should be implemented
# either via target-providing FindLib.cmake:
# ```
# find_package(Threads)
# target_link_libraries(lib Threads::Threads)
# ```
#
# or via INTERFACE libraries:
# add_library(m INTERFACE)
# find_library(M_LIBRARY NAMES m)
# if (M_LIBRARY)
#    target_link_libraries(m ${M_LIBRARY})
# endif ()
# target_link_libraries(m ${M_LIBRARY})
#
# These targets can then be exported along with targets provided by this
# project. Linking against cmake interface targets will handle include
# directories better than raw names given to target_link_libraries

set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)
if (CMAKE_USE_PTHREADS_INIT)
    set(HAVE_PTHREAD TRUE)
    set(ERT_HAVE_THREAD_POOL ON)

    set(pthread "${CMAKE_THREAD_LIBS_INIT}")

    if (THREADS_HAVE_PTHREAD_ARG)
        set(pthreadarg "-pthread")
    endif ()
endif ()

set(dl "${CMAKE_DL_LIBS}")

find_library(M_LIBRARY NAMES m)
if(M_LIBRARY)
    set(m m)
endif ()

find_package(ZLIB)
if (ZLIB_FOUND)
    set(ERT_HAVE_ZLIB ON)
    set(zlib "${ZLIB_LIBRARIES}")
endif ()

find_library(SHLWAPI_LIBRARY NAMES Shlwapi)
if (SHLWAPI_LIBRARY)
    set(shlwapi ${SHLWAPI_LIBRARY})
endif ()

#-----------------------------------------------------------------

# feature tests
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckSymbolExists)
include(CheckTypeSize)

check_function_exists( access HAVE_POSIX_ACCESS)
check_function_exists( _access HAVE_WINDOWS__ACCESS)
check_function_exists( chdir HAVE_POSIX_CHDIR )
check_function_exists( _chdir HAVE_WINDOWS_CHDIR )
check_function_exists( chmod HAVE_CHMOD )
check_function_exists( fnmatch HAVE_FNMATCH )
check_function_exists( fork HAVE_FORK )
check_function_exists( fseeko HAVE_FSEEKO )
check_function_exists( fsync HAVE_FSYNC )
check_function_exists( ftruncate HAVE_FTRUNCATE )
check_function_exists( getcwd HAVE_POSIX_GETCWD)
check_function_exists( _getcwd HAVE_WINDOWS_GETCWD)
check_function_exists( getpwuid HAVE_GETPWUID )
check_function_exists( GetTempPath HAVE_WINDOWS_GET_TEMP_PATH )
check_function_exists( getuid ERT_HAVE_GETUID )
check_function_exists( glob ERT_HAVE_GLOB )
check_function_exists( gmtime_r HAVE_GMTIME_R )
check_function_exists( localtime_r HAVE_LOCALTIME_R )
check_function_exists( lockf ERT_HAVE_LOCKF )
check_function_exists( mkdir HAVE_POSIX_MKDIR)
check_function_exists( _mkdir HAVE_WINDOWS_MKDIR)
check_function_exists( opendir ERT_HAVE_OPENDIR )
check_function_exists( posix_spawn ERT_HAVE_SPAWN )
check_function_exists( readlinkat ERT_HAVE_READLINKAT )
check_function_exists( realpath HAVE_REALPATH )
check_function_exists( regexec ERT_HAVE_REGEXP )
check_function_exists( round HAVE_ROUND )
check_function_exists( symlink ERT_HAVE_SYMLINK )
check_function_exists( timegm HAVE_TIMEGM )
check_function_exists( usleep HAVE__USLEEP )
check_function_exists( unlink HAVE_POSIX_UNLINK )
check_function_exists( _unlink HAVE_WINDOWS_UNLINK )

check_symbol_exists(_tzname time.h HAVE_WINDOWS_TZNAME)
check_symbol_exists( tzname time.h HAVE_TZNAME)

check_include_file(execinfo.h HAVE_EXECINFO)
check_include_file(getopt.h   ERT_HAVE_GETOPT)
check_include_file(unistd.h   ERT_HAVE_UNISTD)

test_big_endian(BIG_ENDIAN)

check_type_size(time_t SIZE_OF_TIME_T)
if (${SIZE_OF_TIME_T} EQUAL 8)
    try_run(RUN_RESULT COMPILE_RESULT ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake/Tests/test_mktime_before1970.c)
    if (${COMPILE_RESULT})
        if (${RUN_RESULT} EQUAL 0)
            set( ERT_TIME_T_64BIT_ACCEPT_PRE1970 ON )
        endif()
    endif()
endif()

try_compile( HAVE_VA_COPY ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake/Tests/test_va_copy.c )
try_compile( HAVE_SIGBUS  ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake/Tests/test_have_sigbus.c )
try_compile( HAVE_PID_T   ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake/Tests/test_pid_t.c )
try_compile( HAVE_MODE_T  ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake/Tests/test_mode_t.c )
try_compile( ERT_HAVE_ISFINITE ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake/Tests/test_isfinite.c)

if (ERT_HAVE_READLINKAT)
  try_compile( ERT_HAVE_READLINKAT_DECLARATION
    ${CMAKE_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/cmake/Tests/test_have_readlinkat_decl.c
    )
endif()

set( BUILD_CXX ON )
try_compile( HAVE_CXX_SHARED_PTR ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake/Tests/test_shared_ptr.cpp )
if (NOT HAVE_CXX_SHARED_PTR)
    set( BUILD_CXX OFF )
endif()

if (HAVE_FORK AND HAVE_PTHREAD AND HAVE_EXECINFO AND HAVE_GETPWUID)
    set( HAVE_UTIL_ABORT_INTERCEPT ON)
    set( HAVE_BACKTRACE ON)
endif()

if (ERT_WINDOWS)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set( ERT_WINDOWS_LFS ON )
    endif()
endif()


find_package(Git)
if(GIT_FOUND)
   execute_process(
     COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
     OUTPUT_VARIABLE GIT_COMMIT
     OUTPUT_STRIP_TRAILING_WHITESPACE
   )

   execute_process(
     COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
     OUTPUT_VARIABLE GIT_COMMIT_SHORT
     OUTPUT_STRIP_TRAILING_WHITESPACE
   )
else()
    set( GIT_COMMIT "unknown (git not found!)")
    set( GIT_COMMIT_SHORT "unknown (git not found!)")
    message( WARNING "Git not found. Build will not contain git revision info." )
endif()

if (ERT_WINDOWS)
   execute_process(COMMAND cmd.exe /c "echo %date% %time%"  OUTPUT_VARIABLE BUILD_TIME)
else() # Linux or Darwin
   execute_process(COMMAND date "+%Y-%m-%d %H:%M:%S" OUTPUT_VARIABLE BUILD_TIME)
endif()
string(STRIP "${BUILD_TIME}" BUILD_TIME)

#-----------------------------------------------------------------

set(INSTALL_GROUP          ""     CACHE STRING "Group to install as - blank to install as current group")

if (MSVC)
   add_definitions( -D__func__="\\"????\\"")
endif()

if (ERT_LINUX)
   add_definitions( -DHAVE_PROC )
endif()

add_subdirectory( external/catch2 )
add_subdirectory( lib )
add_subdirectory( applications )
add_subdirectory( bin )

if (ENABLE_PYTHON AND ERT_WINDOWS)
    if ((NOT MSVC) OR (MSVC_VERSION LESS 1900))
      message(FATAL_ERROR "The Python wrappers require Visual Studio 2015 or newer")
    endif()
endif()

if (ENABLE_PYTHON)
    # If finding the Python interpreter and required packages
    # fails in the python/CMakeLists.txt file the ENABLE_PYTHON
    # will be set to OFF.
    add_subdirectory( python )

    if(RST_DOC)
       add_subdirectory( docs )
    endif()

endif()

if (ENABLE_PYTHON)
   if (NOT ${BUILD_SHARED_LIBS})
      message(FATAL_ERROR "The Python wrappers require shared libraries")
   endif()
endif()

install(EXPORT ecl-config DESTINATION share/cmake/ecl)
export(TARGETS ecl FILE eclConfig.cmake)
export(PACKAGE ecl)
