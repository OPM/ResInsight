cmake_minimum_required( VERSION 2.6 )
project( ERT C CXX )
include( CheckFunctionExists )
ENABLE_TESTING()


if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   set(ERT_LINUX TRUE )
   add_definitions( -DERT_LINUX )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
   set(ERT_WINDOWS TRUE)
   add_definitions( -DERT_WINDOWS )
endif()    


set( CMAKE_C_FLAGS    "-g -O2 -Wall -std=gnu99 -fno-leading-underscore" )
set( CMAKE_CXX_FLAGS  "-g -O2 -Wall" )


set( INSTALL_GROUP          ""     CACHE STRING "Group to install as - blank to install as current group")
if (MSVC)
   set(SHARED_LIB OFF)
else()
   option( SHARED_LIB "Build shared libraries" OFF)
endif()

if (SHARED_LIB)
   set( LIBRARY_TYPE SHARED )
else()
   set( LIBRARY_TYPE STATIC )
endif()     

set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin)

#-----------------------------------------------------------------
find_library( ZLIB_LIBRARY NAMES z )
find_path( ZLIB_HEADER zlib.h /usr/include )

if (ZLIB_LIBRARY AND ZLIB_HEADER)
   option(WITH_ZLIB "Include support for zlib functions compress()/uncompress()" ON)
   if (WITH_ZLIB)
      add_definitions( -DWITH_ZLIB )
   endif()
else()
   set( WITH_ZLIB FALSE )
   message("ZLib not found - zlib support will not be included." )       
endif()
#-----------------------------------------------------------------
find_library( PTHREAD_LIBRARY NAMES pthread )
if (PTHREAD_LIBRARY)
   option( WITH_PTHREAD "Include support for pthreads" ON )
   if (WITH_PTHREAD)
     add_definitions( -DWITH_PTHREAD )
   endif()
else()
   set( WITH_PTHREAD FALSE )
   message("pthread library not found - pthread support will not be included")
endif()
#-----------------------------------------------------------------
find_library( LAPACK_LIBRARY NAMES lapack)
if (LAPACK_LIBRARY)
   set(CMAKE_REQUIRED_LIBS LAPACK_LIBRARY)
   try_compile( BLAS0 ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/CMake/Tests/test_blas.c )
   if (BLAS0)
      set(NEED_BLAS OFF)    
   else()
      set(NEED_BLAS ON)
      find_library( BLAS_LIBRARY NAMES blas)
   endif()
   option(WITH_LAPACK "Build LAPACK enabled code" ON)
   if (WITH_LAPACK)
      add_definitions( -DWITH_LAPACK )
   endif()
else()
   set( WITH_LAPACK OFF)     
   message("LAPACK library not found - LAPACK support will not be included")
endif()
#-----------------------------------------------------------------
find_program(LATEX_PATH NAMES pdflatex)
if (LATEX_PATH)
   option( WITH_LATEX "Build small class for compiling LaTeX files" ON)
   if (WITH_LATEX)
      set( WITH_LATEX ON)
      add_definitions( -DWITH_LATEX )
   endif()
else()
   set( WITH_LATEX OFF )
endif()
#-----------------------------------------------------------------
find_path( EXECINFO_HEADER execinfo.h /usr/include )
if (EXECINFO_HEADER)
  add_definitions( -DHAVE_EXECINFO )
endif()
#-----------------------------------------------------------------
find_path( GETOPT_HEADER getopt.h /usr/include )
if (GETOPT_HEADER)
   add_definitions( -DHAVE_GETOPT )
endif()
#-----------------------------------------------------------------
if (ERT_WINDOWS)
   find_library( SHLWAPI_LIBRARY NAMES Shlwapi )
endif()

if (MSVC)
   add_definitions( -D__func__="\\"????\\"")
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

check_function_exists( usleep HAVE_USLEEP )
if (HAVE_OPENDIR)
  add_definitions( -DHAVE_USLEEP )
endif()

try_compile( HAVE_ISFINITE ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/CMake/Tests/test_isfinite.c )
if (HAVE_ISFINITE)
  add_definitions( -DHAVE_ISFINITE )
endif()

try_compile( MKDIR_POSIX ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/CMake/Tests/test_mkdir.c )
if (MKDIR_POSIX)
  add_definitions( -DMKDIR_POSIX )
endif()

try_compile( HAVE_PID_T ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/CMake/Tests/test_pid_t.c )
if (HAVE_PID_T)
  add_definitions( -DHAVE_PID_T )
endif()

try_compile( HAVE_VA_COPY ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/CMake/Tests/test_va_copy.c )
if (HAVE_VA_COPY)
   add_definitions( -DHAVE_VA_COPY )
endif()


try_compile( ISREG_POSIX ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/CMake/Tests/test_isreg.c )
if (ISREG_POSIX)
  add_definitions( -DHAVE_ISREG )
endif()

if (ERT_LINUX)
   set( NEED_LIBM TRUE )
   set( LINK_STATIC FALSE )
   add_definitions( -DHAVE_PROC )
else()
   set( NEED_LIBM FALSE )
   set( LINK_STATIC TRUE )
endif()


configure_file( ${PROJECT_SOURCE_DIR}/CMake/config/ert_build_config.h.in
                ${PROJECT_BINARY_DIR}/ert_build_config.h )

set( libert_util_build_path   ${PROJECT_BINARY_DIR}/libert_util/src )
set( libert_util_src_path     ${PROJECT_SOURCE_DIR}/libert_util/src )
set( libecl_src_path          ${PROJECT_SOURCE_DIR}/libecl/src )
set( libgeometry_src_path     ${PROJECT_SOURCE_DIR}/libgeometry/src )
set( libwell_src_path         ${PROJECT_SOURCE_DIR}/libwell/src )
set( libplot_src_path         ${PROJECT_SOURCE_DIR}/libplot/src )

include_directories( ${libert_util_build_path}) 
include_directories( ${libert_util_src_path} )

add_subdirectory( libert_util )
add_subdirectory( libgeometry )
add_subdirectory( libecl )
add_subdirectory( libwell )

option( BUILD_ERT      "Build the full ERT application - Linux only" OFF)
option( BUILD_ENS_PLOT "Build small Eclipse plotting program - no" OFF)

#-----------------------------------------------------------------
if (BUILD_ERT)
#-----------------------------------------------------------------

   try_compile( DLOPEN ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/CMake/Tests/test_dlopen.c )
   if (DLOPEN)
      set(NEED_LIBDL OFF)    
   else()
      set(NEED_LIBDL ON)
   endif()

   option(USE_LSF "Include support for LSF" ON)
   set( libconfig_src_path    ${CMAKE_SOURCE_DIR}/libconfig/src )
   set( libsched_src_path     ${CMAKE_SOURCE_DIR}/libsched/src )
   set( librms_src_path       ${CMAKE_SOURCE_DIR}/librms/src )
   set( libanalysis_src_path  ${CMAKE_SOURCE_DIR}/libanalysis/src )
   set( libjob_queue_src_path ${CMAKE_SOURCE_DIR}/libjob_queue/src )
   set( libenkf_src_path      ${CMAKE_SOURCE_DIR}/libenkf/src )

   add_subdirectory( libconfig )
   add_subdirectory( libanalysis )
   add_subdirectory( librms )
   add_subdirectory( libsched )
   add_subdirectory( libjob_queue )
   add_subdirectory( libplot )
   add_subdirectory( libenkf)

   #add_dependencies( config_shared util_shared )
   ##add_dependencies( analysis_shared util_shared )
   #add_dependencies( rms_shared geometry_shared util_shared )
   #add_dependencies( sched_shared util_shared ecl_shared )
   #add_dependencies( job_queue_shared config_shared util )
   #add_dependencies( plot_shared util_shared )
   #add_dependencies( enkf_shared plot_shared ecl_shared util_shared config_shared sched_shared rms_shared analysis job_queue_shared )
   #add_dependencies( ert_shared enkf_shared plot_shared )
   #
   #add_dependencies( config_static util_static )
   ##add_dependencies( analysis_static util_static )
   #add_dependencies( rms_static geometry_static util_static )
   #add_dependencies( sched_static util_static ecl_static )
   #add_dependencies( job_queue_static config_static util )
   #add_dependencies( plot_static util_static )
   #add_dependencies( enkf_static plot_static ecl_static util_static config_static sched_static rms_static analysis job_queue_static )
   #add_dependencies( ert enkf_static plot_static )
endif()

