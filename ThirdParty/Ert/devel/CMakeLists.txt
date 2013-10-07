cmake_minimum_required( VERSION 2.6 )
project( ERT C CXX )

set( ERT_VERSION_MAJOR 1 )
set( ERT_VERSION_MINOR 0 )

option( BUILD_ERT           "Build the full ERT application - Linux only" OFF)
option( BUILD_ENS_PLOT      "Build small Eclipse plotting program - no" OFF)
option( BUILD_TESTS         "Should the tests be built" OFF)
option( BUILD_APPLICATIONS  "Should we build small utility applications" OFF)
option( BUILD_ECL_SUMMARY   "Build the commandline application ecl_summary" OFF)
option( BUILD_PYTHON        "Run py_compile on the python wrappers" OFF)
option( BUILD_SHARED_LIBS   "Build shared libraries" ON )
option( INSTALL_ERT         "Should anything be installed when issuing make install?" ON)
option( ERT_BUILD_GUI       "Should the pyQt based gui be compiled and installed" OFF )
option( ERT_USE_OPENMP      "Use OpenMP - currently only in EclGrid" ON)

include( CheckFunctionExists )
include( CheckTypeSize )
ENABLE_TESTING()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   set(ERT_LINUX TRUE )
   add_definitions( -DERT_LINUX )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
   set(ERT_WINDOWS TRUE)
   add_definitions( -DERT_WINDOWS )
#  This symbol must be defined to support large (> 2GB) files on windows.
   add_definitions( -DWINDOWS_LFS )
endif()    


# Treat warnings as errors if not on Windows
if (ERT_WINDOWS)
   set( CMAKE_C_FLAGS    "-O2" )
   set( CMAKE_CXX_FLAGS  "-O2" )
else()
   set( CMAKE_C_FLAGS    "-std=gnu99 -g -O2 -Wall -Wno-unknown-pragmas " )
   set( CMAKE_CXX_FLAGS  "-g -Wall -O2 ")
endif()

if (ERT_USE_OPENMP)
   find_package(OpenMP)
   if (OPENMP_FOUND) 
       message(STATUS "Enabling OpenMP support")
       set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
       set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
       set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
       set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${OpenMP_SHARED_LINKER_FLAGS}")
   else()
       message(STATUS "OpenMP package not found - OpenMP disabled")
   endif()
endif()


include(cmake/ert_check.cmake)
include(cmake/ert_find.cmake)
include(cmake/Modules/UseMultiArch.cmake)
include(cmake/ert_link.cmake)


set(INSTALL_GROUP          ""     CACHE STRING "Group to install as - blank to install as current group")
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin)


if (BUILD_SHARED_LIBS)
   set( LIBRARY_TYPE SHARED )
else()
   set( LIBRARY_TYPE STATIC )
   if (BUILD_ERT)
      message(FATAL_ERROR "The full ERT application must be built with shared libraries")
   endif()
   if (BUILD_PYTHON)
      message(FATAL_ERROR "The Python wrappers require shared libraries")
   endif()
endif()

if (MSVC)
   add_definitions( -D__func__="\\"????\\"")
endif()


if (ERT_LINUX)
   set( NEED_LIBM TRUE )
   set( LINK_STATIC FALSE )
   add_definitions( -DHAVE_PROC )
else()
   set( NEED_LIBM FALSE )
   set( LINK_STATIC TRUE )
endif()


configure_file( ${PROJECT_SOURCE_DIR}/cmake/config/ert_build_config.h.in
                ${PROJECT_BINARY_DIR}/ert_build_config.h )

include_directories( ${PROJECT_SOURCE_DIR}/libert_util/include )
include_directories( ${PROJECT_BINARY_DIR}/libert_util/include )
if (MSVC)
  include_directories( ${PROJECT_BINARY_DIR}/libert_util/include/ert/util )
endif()
add_subdirectory( libert_util )


include_directories( ${PROJECT_SOURCE_DIR}/libgeometry/include )
add_subdirectory( libgeometry )

if (BUILD_ERT OR BUILD_ENS_PLOT)
   include_directories( ${PROJECT_SOURCE_DIR}/libplot/include )
   add_subdirectory( libplot )
endif()

include_directories( ${PROJECT_SOURCE_DIR}/libecl/include )
add_subdirectory( libecl )

include_directories( ${PROJECT_SOURCE_DIR}/libecl_well/include )
add_subdirectory( libecl_well )


#-----------------------------------------------------------------
if (BUILD_ERT)
#-----------------------------------------------------------------
   include_directories( ${PROJECT_SOURCE_DIR}/libconfig/include )
   add_subdirectory( libconfig )

   include_directories( ${PROJECT_SOURCE_DIR}/libsched/include )
   add_subdirectory( libsched )

   include_directories( ${PROJECT_SOURCE_DIR}/librms/include )
   add_subdirectory( librms )

   include_directories( ${PROJECT_SOURCE_DIR}/libanalysis/include )
   add_subdirectory( libanalysis )

   include_directories( ${PROJECT_SOURCE_DIR}/libjob_queue/include )
   add_subdirectory( libjob_queue )

   include_directories( ${PROJECT_SOURCE_DIR}/libenkf/include )
   add_subdirectory( libenkf )

   install(DIRECTORY ${PROJECT_SOURCE_DIR}/share DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()


if (BUILD_PYTHON)
   include(cmake/python.cmake2)
   add_subdirectory( python )
endif()

