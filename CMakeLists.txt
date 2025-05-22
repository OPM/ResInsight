# The way boost is configured and used in Boost 1.86 requires CMake 3.26
cmake_minimum_required(VERSION 3.26)

# Check if CMake version is 4.0 or higher
if(CMAKE_VERSION VERSION_GREATER_EQUAL 4.0)
  message(
    FATAL_ERROR
      "CMake version ${CMAKE_VERSION} detected. We are investigating issues for CMake 4. Use CMake 3.x to build ResInsight."
  )
endif()

cmake_policy(SET CMP0020 NEW)
if(POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif()
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 OLD)
endif()
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 OLD)
endif()
if(POLICY CMP0169)
  cmake_policy(SET CMP0169 OLD)
endif()

include(CheckCSourceCompiles)
include(FetchContent)

project(ResInsight)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13.0)
    message(
      FATAL_ERROR
        "Minimum GCC version is 13, detected version: ${CMAKE_CXX_COMPILER_VERSION}"
    )
  endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.0)
    message(
      FATAL_ERROR
        "Minimum supported version is Clang 19, detected version: ${CMAKE_CXX_COMPILER_VERSION}"
    )
  endif()
endif()

# Ensure all binary files ensd up in the same folder as the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(VIZ_MODULES_FOLDER_NAME Fwk/VizFwk)

message(STATUS "Set CEE_USE_QT6 to ON")
set(CEE_USE_QT5 OFF)
set(CEE_USE_QT6 ON)

if(UNIX)
  option(
    RESINSIGHT_PREFER_LEGACY_OPENGL
    "Link with Legacy OpenGL libraries. This may be necessary in some virtualization environments"
    ON
  )
  if(RESINSIGHT_PREFER_LEGACY_OPENGL)
    set(OpenGL_GL_PREFERENCE LEGACY)
  endif()
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(BUILD_SHARED_LIBS
    OFF
    CACHE BOOL "ERT: Build shared libraries"
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

option(RESINSIGHT_BUNDLE_TESTMODELS "Copy TestModels into the installation" OFF)
mark_as_advanced(RESINSIGHT_BUNDLE_TESTMODELS)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ##############################################################################
# Setup the main platform defines
# ##############################################################################
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_definitions(-DCVF_LINUX)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_definitions(-DCVF_OSX)
elseif(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  set(_HAS_STD_BYTE 0)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS
        "-DCVF_LINUX -pipe -Wextra -Woverloaded-virtual -Wformat -Wno-unused-parameter"
    )

  else()
    set(CMAKE_CXX_FLAGS
        "-DCVF_LINUX -pipe -Wextra -Woverloaded-virtual -Wformat -Wno-unused-parameter -Wno-array-bounds -Wno-dangling-reference -Wno-stringop-overflow"
    )
  endif()
  set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -g3 -O0 -DDEBUG -D_DEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNO_DEBUG")
endif()

# ##############################################################################
# OpenMP
# ##############################################################################
option(RESINSIGHT_USE_OPENMP "Enable OpenMP parallellization in the code" ON)
if(RESINSIGHT_USE_OPENMP)
  find_package(OpenMP)
  if(OPENMP_FOUND)
    message(STATUS "Enabling OpenMP support")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} -DUSE_OPENMP")
    set(CMAKE_EXE_LINKER_FLAGS
        "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}"
    )
  else()
    message(STATUS "Disabling OpenMP support")
  endif()
endif()

# ##############################################################################
# buildcache
# ##############################################################################
find_program(buildcache_program buildcache)
if(buildcache_program)
  set(CMAKE_CXX_COMPILER_LAUNCHER "${buildcache_program}")
  message(STATUS "Found buildcache from : ${buildcache_program}")

  if(MSVC)
    # New concept for defining MSVC debug info flags introduced in 3.25.
    # Evaluate this concept to simplify the setting of CXX-flags
    # https://cmake.org/cmake/help/v3.25/prop_tgt/MSVC_DEBUG_INFORMATION_FORMAT.html#prop_tgt:MSVC_DEBUG_INFORMATION_FORMAT
    # https://cmake.org/cmake/help/v3.25/policy/CMP0141.html#policy:CMP0141

    # It is required to use compiler flag /Z7 to be able to use buildcache
    # https://github.com/mbitsnbites/buildcache/blob/master/doc/usage.md
    # https://learn.microsoft.com/en-us/cpp/build/reference/z7-zi-zi-debug-information-format?view=msvc-170

    string(REGEX REPLACE "/Zi" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string(REGEX REPLACE "/Zi" "" CMAKE_CXX_FLAGS_DEBUG
                         "${CMAKE_CXX_FLAGS_DEBUG}"
    )
    string(REGEX REPLACE "/Zi" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO
                         "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}"
    )

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Z7")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /Z7")

    # Add this to avoid the warning: overriding '/Zi' with '/Z7'
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
        "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>"
    )
  endif()
else()
  message(STATUS "No buildcache found")
endif()

# ##############################################################################
# ABAQUS ODB
# ##############################################################################
set(RESINSIGHT_ODB_API_DIR
    ""
    CACHE
      PATH
      "Optional path to the ABAQUS ODB API from Simulia. Needed for support of geomechanical models"
)

option(RESINSIGHT_DOWNLOAD_ODB_FROM_SERVER
       "Download ODB library from private repository" OFF
)
mark_as_advanced(FORCE RESINSIGHT_DOWNLOAD_ODB_FROM_SERVER)
set(RESINSIGHT_ODB_VERSION
    "2020"
    CACHE STRING "Abaqus API version to use, 2020 or 2024"
)
mark_as_advanced(FORCE RESINSIGHT_ODB_VERSION)

if(RESINSIGHT_DOWNLOAD_ODB_FROM_SERVER)

  if(MSVC)
    FetchContent_Declare(
      odb-library-from-server
      URL http://10.10.0.26:8080/job/resinsight-dependencies/ws/${RESINSIGHT_ODB_VERSION}/odb_api_win64.zip
    )
  else()
    FetchContent_Declare(
      odb-library-from-server
      URL http://10.10.0.26:8080/job/resinsight-dependencies/ws/${RESINSIGHT_ODB_VERSION}/odb_api_linux.zip
    )
  endif()

  FetchContent_MakeAvailable(odb-library-from-server)
  set(RESINSIGHT_ODB_API_DIR ${odb-library-from-server_SOURCE_DIR})
endif()

if(NOT ${RESINSIGHT_ODB_API_DIR} EQUAL "")
  add_definitions(-DUSE_ODB_API)
  set(RESINSIGHT_USE_ODB_API 1)
  message(
    STATUS
      "Using ODB-Api ${RESINSIGHT_ODB_VERSION} from : ${RESINSIGHT_ODB_API_DIR}"
  )
  if(MSVC)
    add_definitions(-D_WINDOWS_SOURCE)
  else()
    add_definitions(-D_LINUX_SOURCE)
  endif()
endif()

# ##############################################################################
# Version number
# ##############################################################################
if(UNIX AND NOT APPLE)
  if(EXISTS "/etc/os-release")
    # Read the contents of /etc/os-release
    file(READ "/etc/os-release" OS_RELEASE_CONTENT)

    # Extract ID
    string(REGEX MATCH "PRETTY_NAME=\"?([^\n\"]+)\"?" _ ${OS_RELEASE_CONTENT})
    set(RESINSIGHT_BUILD_SYSTEM_ID ${CMAKE_MATCH_1})

    # Display the extracted values
    message(STATUS "Operating System ID: ${RESINSIGHT_BUILD_SYSTEM_ID}")
  else()
    message(WARNING "/etc/os-release not found. Cannot determine OS details.")
  endif()
endif()

include(ResInsightVersion.cmake)

# ##############################################################################
# Octave
# ##############################################################################
find_package(Octave)

# ##############################################################################
# Grpc
# ##############################################################################
option(RESINSIGHT_ENABLE_GRPC "Enable the gRPC scripting framework" OFF)
if(RESINSIGHT_ENABLE_GRPC)
  option(
    RESINSIGHT_GRPC_DOWNLOAD_PYTHON_MODULE
    "Download the gRPC python modules to enable generation of Python interface"
    ON
  )
  option(RESINSIGHT_GRPC_BUNDLE_PYTHON_MODULE
         "Bundle the gRPC python modules into the install folder" OFF
  )
  add_definitions(-DENABLE_GRPC)
endif()

# ##############################################################################
# Unity Build
# ##############################################################################

# CMAKE_UNITY_BUILD was introduced in CMake 3.16.2
option(RESINSIGHT_ENABLE_UNITY_BUILD
       "Experimental speedup of compilation using CMake Unity Build" OFF
)
mark_as_advanced(FORCE RESINSIGHT_ENABLE_UNITY_BUILD)
if(RESINSIGHT_ENABLE_UNITY_BUILD)
  set(CAF_ENABLE_UNITY_BUILD true)
  set(CVF_ENABLE_UNITY_BUILD true)
endif()

# ##############################################################################
# resdata
# ##############################################################################

# Use of CMAKE_CXX_COMPILER_LAUNCHER is not working with resdata. Disable by
# temporarily setting compiler launcher to nothing
set(TEMP_CMAKE_CXX_COMPILER_LAUNCHER ${CMAKE_CXX_COMPILER_LAUNCHER})
set(CMAKE_CXX_COMPILER_LAUNCHER)

if(NOT MSVC)
  # Linux: Optional configuration of externally installed ERT, requires path to
  # libraries and includes
  set(RESINSIGHT_ERT_EXTERNAL_LIB_ROOT
      ""
      CACHE PATH "Path to installed ERT libraries"
  )
  set(RESINSIGHT_ERT_EXTERNAL_INCLUDE_ROOT
      ""
      CACHE PATH "Path to installed ERT includes"
  )
endif()

if(RESINSIGHT_ERT_EXTERNAL_LIB_ROOT OR RESINSIGHT_ERT_EXTERNAL_INCLUDE_ROOT)
  if(NOT (RESINSIGHT_ERT_EXTERNAL_LIB_ROOT
          AND RESINSIGHT_ERT_EXTERNAL_INCLUDE_ROOT)
  )
    message(
      FATAL_ERROR
        "Both RESINSIGHT_ERT_EXTERNAL_LIB_ROOT and RESINSIGHT_ERT_EXTERNAL_INCLUDE_ROOT must be defined"
    )
  endif()

  message(
    FATAL_ERROR
      "TODO: Building using and external system installed ERT is broken."
  )

  list(APPEND ERT_INCLUDE_DIRS ${RESINSIGHT_ERT_EXTERNAL_INCLUDE_ROOT})

  list(APPEND ERT_LIBRARIES ${RESINSIGHT_ERT_EXTERNAL_LIB_ROOT}/resdata.so)

else()

  # Disable install of ERT libs and headers, as Ert code is compiled and linked
  # directly
  set(INSTALL_ERT
      OFF
      CACHE BOOL "ERT: Install library"
  )
  set(ERT_USE_OPENMP
      ${OPENMP_FOUND}
      CACHE BOOL "ERT: Compile using OpenMP"
  )

  # Remember original state
  set(ORIGINAL_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})

  if(MSVC)
    # Force static linking on Windows
    set(BUILD_SHARED_LIBS OFF)
    set(ERT_HAVE_UNISTD OFF) # If anyone has mingw installed
  else()
    set(RESINSIGHT_ERT_EXTERNAL_SOURCE_ROOT
        ""
        CACHE STRING "Path to ERT CMakeList.txt (source path)"
    )

    # Force dynamic linking on other platforms Copy of libraries into install
    # folder of ResInsight is done a bit further down in this file
    set(BUILD_SHARED_LIBS ON)
  endif()

  if(RESINSIGHT_ERT_EXTERNAL_SOURCE_ROOT)
    add_subdirectory(
      ${RESINSIGHT_ERT_EXTERNAL_SOURCE_ROOT} ${CMAKE_BINARY_DIR}/ThirdParty/Ert
    )
  else()
    add_subdirectory(ThirdParty/Ert)
  endif()

  if(MSVC)
    # resdata : Disable some warnings
    set_target_properties(
      resdata
      PROPERTIES
        COMPILE_FLAGS
        "/wd4244 /wd4267 /wd4013 /wd4190 /wd4018 /wd4477 /wd4098 /wd4293 /wd4305 /wd4020 /wd4028 /wd4715 /wd4245 /wd4804 /wd4100 /wd4456 /wd4458 /wd4090 /wd4297 /wd4701 /wd4101 /wd4702 /wd4457"
    )
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set_target_properties(
      resdata
      PROPERTIES
        COMPILE_FLAGS
        "-Wno-deprecated -Wno-deprecated-declarations -Wno-sign-compare"
    )
  else()
    set_target_properties(
      resdata
      PROPERTIES
        COMPILE_FLAGS
        "-Wno-deprecated -Wno-deprecated-declarations -Wno-clobbered -Wno-int-in-bool-context"
    )

    target_compile_options(
      resdata PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-Wno-class-memaccess>
    )

  endif()

  list(APPEND THIRD_PARTY_LIBRARIES resdata)

  set_property(TARGET catch2 PROPERTY FOLDER "Thirdparty")

  # Restore original state
  set(BUILD_SHARED_LIBS ${ORIGINAL_BUILD_SHARED_LIBS})

endif(RESINSIGHT_ERT_EXTERNAL_LIB_ROOT OR RESINSIGHT_ERT_EXTERNAL_INCLUDE_ROOT)

# restore buildcache when resdata has been configured
set(CMAKE_CXX_COMPILER_LAUNCHER ${TEMP_CMAKE_CXX_COMPILER_LAUNCHER})

# ##############################################################################
# Init GIT submodules if they haven't already #
# ##############################################################################
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
  execute_process(
    COMMAND git log -1 --pretty=format:%h
    OUTPUT_VARIABLE RESINSIGHT_GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET
  )

  # Update submodules as needed
  option(RESINSIGHT_UPDATE_SUBMODULES "Check submodules during build" ON)
  mark_as_advanced(RESINSIGHT_UPDATE_SUBMODULES)

  if(RESINSIGHT_UPDATE_SUBMODULES)
    set(SUBDIR "${PROJECT_SOURCE_DIR}/ThirdParty")
    message(STATUS "Initializing GIT submodules")
    execute_process(
      COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive -- ${SUBDIR}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      RESULT_VARIABLE GIT_SUBMOD_RESULT
    )
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
      message(
        FATAL_ERROR
          "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules"
      )
    endif()
  endif()
endif()

# ##############################################################################
# OpenVDS seismic file access
# ##############################################################################

message(STATUS "Starting download of external library OpenVDS ...")

if(MSVC)
  FetchContent_Declare(
    openvds
    URL https://downloads.bluware.com/artifactory/Releases-OpenVDSPlus/3.4/openvds+-3.4.5-win.zip
  )
else()
  FetchContent_Declare(
    openvds
    URL https://downloads.bluware.com/artifactory/Releases-OpenVDSPlus/3.4/openvds+-3.4.5-manylinux_2014.tar.gz
  )
endif()

FetchContent_MakeAvailable(openvds)
message(STATUS "... completed download of external library OpenVDS")

set(RESINSIGHT_OPENVDS_API_DIR ${openvds_SOURCE_DIR})
message(STATUS "Using OpenVDS api from : ${RESINSIGHT_OPENVDS_API_DIR}")

if(MSVC)
  list(APPEND EXTERNAL_LINK_LIBRARIES
       ${RESINSIGHT_OPENVDS_API_DIR}/lib/msvc_141/openvds.lib
  )
else()
  list(APPEND EXTERNAL_LINK_LIBRARIES
       ${RESINSIGHT_OPENVDS_API_DIR}/lib64/libopenvds.so
  )
endif()

# ##############################################################################
# HDF5
# ##############################################################################

if(MSVC)
  # this option is disabled in the daily build defined in
  # ResInsightWithCache.yml, as the text input to ninja becomes too long
  option(RESINSIGHT_ENABLE_HDF5 "Use external HDF5 library" ON)

  if(RESINSIGHT_ENABLE_HDF5)
    FetchContent_Declare(
      hdf-external-lib
      URL https://github.com/CeetronSolutions/resinsight-dependencies/releases/download/2023.04/HDF_Group.zip
    )
    FetchContent_MakeAvailable(hdf-external-lib)
    set(RESINSIGHT_HDF5_DIR ${hdf-external-lib_SOURCE_DIR}/HDF5/1.8.18)
  endif()

  if(NOT ${RESINSIGHT_HDF5_DIR} EQUAL "")
    list(APPEND EXTERNAL_LINK_LIBRARIES ${RESINSIGHT_HDF5_DIR}/lib/hdf5.lib
         ${RESINSIGHT_HDF5_DIR}/lib/hdf5_cpp.lib
    )

    set(RESINSIGHT_FOUND_HDF5 1)
    message(STATUS "Using HDF5 from : ${RESINSIGHT_HDF5_DIR}")
  else()
    message(
      WARNING
        "Use of HDF5 is enabled, but RESINSIGHT_HDF5_DIR is empty. Specify RESINSIGHT_HDF5_DIR to be able to use HDF5"
    )
  endif()
else()
  if(RESINSIGHT_ENABLE_HDF5)
    find_package(HDF5 COMPONENTS CXX)
    if(HDF5_FOUND)
      list(APPEND EXTERNAL_LINK_LIBRARIES ${HDF5_LIBRARIES})

      set(RESINSIGHT_FOUND_HDF5 1)
      message(STATUS "Using HDF5 libraries : ${HDF5_LIBRARIES}")

      option(RESINSIGHT_HDF5_BUNDLE_LIBRARIES "Bundle HDF5 libraries" OFF)
      mark_as_advanced(FORCE RESINSIGHT_HDF5_BUNDLE_LIBRARIES)
      if(RESINSIGHT_HDF5_BUNDLE_LIBRARIES)
        message(STATUS "Bundling of HDF5 libraries is enabled")
      endif() # RESINSIGHT_HDF5_BUNDLE_LIBRARIES

    else()
      message(WARNING "Use of HDF5 is enabled, but no HDF5 is found.")
    endif() # HDF5_FOUND
  endif()
endif() # MSVC

# ##############################################################################
# Opm
# ##############################################################################

add_subdirectory(ThirdParty/custom-opm-flowdiagnostics)
add_subdirectory(ThirdParty/custom-opm-flowdiag-app)

# CREATE_OPM_COMMON_KEYWORDS is used to create c++-code for the keywords in
# opm-common JSON files See description in
# ThirdParty/custom-opm-common/CMakeLists.txt When changing this flag, it is
# required to reopen the Visual Studio project
set(CREATE_OPM_COMMON_KEYWORDS OFF)

add_subdirectory(ThirdParty/custom-opm-common)

if(NOT CREATE_OPM_COMMON_KEYWORDS)
  add_subdirectory(ThirdParty/custom-opm-common/custom-opm-parser-tests)
  list(APPEND OPM_LIBRARIES custom-opm-common)
  set_property(TARGET opm-parser-tests PROPERTY FOLDER "Thirdparty/OPM")
endif()

list(APPEND OPM_LIBRARIES custom-opm-flowdiagnostics custom-opm-flowdiag-app)
set_property(TARGET ${OPM_LIBRARIES} PROPERTY FOLDER "Thirdparty/OPM")

# ##############################################################################
# NRLib
# ##############################################################################

add_subdirectory(ThirdParty/NRLib)

list(APPEND THIRD_PARTY_LIBRARIES NRLib)

# ##############################################################################
# openzgy
# ##############################################################################

add_definitions(-DOPENZGY_STATIC)
add_subdirectory(ThirdParty/openzgy)

list(APPEND THIRD_PARTY_LIBRARIES openzgy)
set_property(TARGET zfp PROPERTY FOLDER "Thirdparty")
if(TARGET openzgy-tests)
  set_property(TARGET openzgy-tests PROPERTY FOLDER "Thirdparty")
endif()

# ##############################################################################
# Qt
# ##############################################################################

find_package(
  Qt6
  COMPONENTS
  REQUIRED
  Core
  Gui
  OpenGL
  Network
  Widgets
  Charts
)
set(QT_LIBRARIES Qt6::Core Qt6::Gui Qt6::OpenGL Qt6::Network Qt6::Widgets
                 Qt6::Charts
)
qt_standard_project_setup()

# Disable use of foreach
add_definitions(-DQT_NO_FOREACH)

# Open GL
find_package(OpenGL)

# ##############################################################################
# QtCharts
# ##############################################################################
set(RESINSIGHT_USE_QT_CHARTS ON)
add_definitions(-DUSE_QTCHARTS)

# ##############################################################################
# Qwt
# ##############################################################################
set(USE_QT6 ON)
add_subdirectory(ThirdParty/qwt)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(
    qwt PRIVATE -Wno-deprecated-copy -Wno-deprecated-enum-float-conversion
                -Wno-deprecated-enum-enum-conversion
  )
elseif(MSVC)
  target_compile_options(qwt PRIVATE /wd4996 /wd4005)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(qwt PRIVATE -Wno-deprecated)
endif()

list(APPEND THIRD_PARTY_LIBRARIES qwt)

# ##############################################################################
# Qt Advanced Docking System
# ##############################################################################
set(ADS_VERSION "4.2.1")
set(QT_VERSION_MAJOR 6)

add_subdirectory(ThirdParty/qtadvanceddocking EXCLUDE_FROM_ALL)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(qt6advanceddocking PRIVATE -Wno-deprecated-copy)
elseif(MSVC)
  target_compile_options(qt6advanceddocking PRIVATE /wd4996 /wd4005)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(qt6advanceddocking PRIVATE -Wno-deprecated)
endif()

list(APPEND THIRD_PARTY_LIBRARIES qt6advanceddocking)

# ##############################################################################
# Nightcharts
# ##############################################################################

add_subdirectory(ThirdParty/nightcharts)
if(MSVC)
  target_compile_options(nightcharts PRIVATE /wd4996)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(nightcharts PRIVATE -Wno-deprecated)
endif()

list(APPEND THIRD_PARTY_LIBRARIES nightcharts)

# ##############################################################################
# C++ Mathematical Expression Parsing And Evaluation Library
# ##############################################################################

add_subdirectory(ThirdParty/expressionparser)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(expressionparser PUBLIC -Wno-overloaded-virtual)
endif()

list(APPEND THIRD_PARTY_LIBRARIES expressionparser)

# ##############################################################################
# clipper
# ##############################################################################

add_subdirectory(ThirdParty/clipper)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(clipper PUBLIC -Wno-deprecated-copy)
endif()

list(APPEND THIRD_PARTY_LIBRARIES clipper)

# ##############################################################################
# roffcpp
# ##############################################################################

add_subdirectory(ThirdParty/roffcpp)
list(APPEND THIRD_PARTY_LIBRARIES roffcpp)
set_property(TARGET roffcpp-tests gtest gtest_main PROPERTY FOLDER "Thirdparty")

# ##############################################################################
# regression-analysis
# ##############################################################################

set(REGRESSION_FETCH_EIGEN false)
add_subdirectory(ThirdParty/regression-analysis)
list(APPEND THIRD_PARTY_LIBRARIES regression-analysis)
set_property(
  TARGET regression-analysis-tests gtest gtest_main PROPERTY FOLDER
                                                             "Thirdparty"
)

# ##############################################################################
# surfio
# ##############################################################################

add_subdirectory(ThirdParty/custom-surfio)
list(APPEND THIRD_PARTY_LIBRARIES surfio_lib)

# ##############################################################################
# toml++
# ##############################################################################

add_subdirectory(ThirdParty/tomlplusplus)

# ##############################################################################
# spdlog and fast_float
# ##############################################################################

find_package(FastFloat CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
list(APPEND THIRD_PARTY_LIBRARIES FastFloat::fast_float spdlog::spdlog)

# ##############################################################################
# Arrow
# ##############################################################################
find_package(Arrow CONFIG REQUIRED)
if(MSVC)
  list(APPEND THIRD_PARTY_LIBRARIES Arrow::arrow_shared)
else()
  list(APPEND THIRD_PARTY_LIBRARIES Arrow::arrow_static)
endif()

# ##############################################################################
# Parquet
# ##############################################################################
find_package(Parquet CONFIG REQUIRED)
if(MSVC)
  list(APPEND THIRD_PARTY_LIBRARIES Parquet::parquet_shared)
else()
  list(APPEND THIRD_PARTY_LIBRARIES Parquet::parquet_static)
endif()

# ##############################################################################
# pugixml
# ##############################################################################
find_package(pugixml CONFIG REQUIRED)
if(MSVC)
  list(APPEND THIRD_PARTY_LIBRARIES pugixml::pugixml)
else()
  list(APPEND THIRD_PARTY_LIBRARIES pugixml::static)
endif()

# ##############################################################################
# nonstd::type-lite
# ##############################################################################
find_package(type-lite CONFIG REQUIRED)
list(APPEND THIRD_PARTY_LIBRARIES nonstd::type-lite)

# ##############################################################################
# Thirdparty libraries are put in ThirdParty solution folder
# ##############################################################################
set_property(TARGET ${THIRD_PARTY_LIBRARIES} PROPERTY FOLDER "Thirdparty")

# ##############################################################################
# Build list of DLLs needed for executables
# ##############################################################################
if(MSVC)

  if(NOT ${RESINSIGHT_ODB_API_DIR} EQUAL "")
    set(RESINSIGHT_USE_ODB_API 1)
  endif()

  # Odb Dlls
  if(RESINSIGHT_USE_ODB_API)
    # Find all the dlls
    file(GLOB RI_ALL_ODB_DLLS ${RESINSIGHT_ODB_API_DIR}/lib/*.dll)

    # Strip off the path
    foreach(aDLL ${RI_ALL_ODB_DLLS})
      get_filename_component(filenameWithExt ${aDLL} NAME)
      list(APPEND RI_ODB_DLLS ${filenameWithExt})
    endforeach(aDLL)

    foreach(aDLL ${RI_ODB_DLLS})
      list(APPEND RI_FILENAMES ${RESINSIGHT_ODB_API_DIR}/lib/${aDLL})
    endforeach()
  endif()

  # OpenVDS Dlls
  set(OPENVDS_DLL_NAMES openvds segyutils)
  foreach(OPENVDS_DLL_NAME ${OPENVDS_DLL_NAMES})
    list(APPEND RI_FILENAMES
         ${RESINSIGHT_OPENVDS_API_DIR}/bin/msvc_141/${OPENVDS_DLL_NAME}.dll
    )
  endforeach(OPENVDS_DLL_NAME)
  list(APPEND RI_FILENAMES
       ${RESINSIGHT_OPENVDS_API_DIR}/bin/msvc_141/SEGYImport.exe
  )

  # HDF5 Dlls
  if(RESINSIGHT_FOUND_HDF5)
    set(HDF5_DLL_NAMES hdf5 hdf5_cpp szip zlib)
    foreach(HDF5_DLL_NAME ${HDF5_DLL_NAMES})
      list(APPEND RI_FILENAMES ${RESINSIGHT_HDF5_DIR}/bin/${HDF5_DLL_NAME}.dll)
    endforeach(HDF5_DLL_NAME)
  endif()

else()
  # Linux

  # OpenVDS lib files
  list(APPEND RI_FILENAMES ${RESINSIGHT_OPENVDS_API_DIR}/bin/SEGYImport)

  set(OPENVDS_LIB_NAMES
      libopenvds.so
      libopenvds.so.3
      libopenvds.so.3.4.5
      libopenvds-85fcb584.so.3.4.5
      libsegyutils.so
      libsegyutils.so.3
      libsegyutils.so.3.4.5
  )
  foreach(OPENVDS_LIB_NAME ${OPENVDS_LIB_NAMES})
    list(APPEND RI_FILENAMES
         ${RESINSIGHT_OPENVDS_API_DIR}/lib64/${OPENVDS_LIB_NAME}
    )
  endforeach(OPENVDS_LIB_NAME)
endif(MSVC)

# ##############################################################################
# Unity Build
# ##############################################################################
if(RESINSIGHT_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : qwt")
  set_property(TARGET qwt PROPERTY UNITY_BUILD true)
  message("Cmake Unity build is enabled on : custom-opm-flowdiagnostics")
  set_property(TARGET custom-opm-flowdiagnostics PROPERTY UNITY_BUILD true)
  # message("Cmake Unity build is enabled on : custom-opm-common")
  # set_property(TARGET custom-opm-common PROPERTY UNITY_BUILD true)
  message("Cmake Unity build is enabled on : qtadvanceddocking")
  set_property(TARGET qt6advanceddocking PROPERTY UNITY_BUILD true)
endif()

# ##############################################################################
# Vizualization Framework
# ##############################################################################

add_subdirectory(${VIZ_MODULES_FOLDER_NAME}/LibCore)
add_subdirectory(${VIZ_MODULES_FOLDER_NAME}/LibGeometry)
add_subdirectory(${VIZ_MODULES_FOLDER_NAME}/LibRender)
add_subdirectory(${VIZ_MODULES_FOLDER_NAME}/LibViewing)
add_subdirectory(${VIZ_MODULES_FOLDER_NAME}/LibGuiQt)

list(
  APPEND
  VIZ_FWK_LIBRARIES
  LibGuiQt
  LibViewing
  LibRender
  LibGeometry
  LibCore
)

if(MSVC)
  target_compile_options(LibGuiQt PRIVATE /wd4996)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(
    LibRender PRIVATE -Wno-undefined-var-template -Wno-invalid-source-encoding
                      -Wno-null-pointer-arithmetic
  )
endif()

set_property(TARGET ${VIZ_FWK_LIBRARIES} PROPERTY FOLDER "VizFwk")
if(WIN32)
  set_property(TARGET run_Glsl2Include PROPERTY FOLDER "VizFwk")
endif()

# ##############################################################################
# Application Framework
# ##############################################################################

add_subdirectory(Fwk/AppFwk/cafAnimControl)
add_subdirectory(Fwk/AppFwk/cafViewer)

add_subdirectory(Fwk/AppFwk/cafProjectDataModel/cafPdmCore)
add_subdirectory(Fwk/AppFwk/cafProjectDataModel/cafPdmUiCore)
add_subdirectory(Fwk/AppFwk/cafProjectDataModel/cafPdmXml)
add_subdirectory(Fwk/AppFwk/cafProjectDataModel)
add_subdirectory(Fwk/AppFwk/cafDataLoader)

add_subdirectory(Fwk/AppFwk/cafCommand)
add_subdirectory(Fwk/AppFwk/cafUserInterface)
add_subdirectory(Fwk/AppFwk/cafPdmCvf)

add_subdirectory(Fwk/AppFwk/CommonCode)
add_subdirectory(Fwk/AppFwk/cafVizExtensions)

add_subdirectory(Fwk/AppFwk/cafPdmScripting)
add_subdirectory(Fwk/AppFwk/cafCommandFeatures)

add_subdirectory(Fwk/AppFwk/cafTensor)
add_subdirectory(Fwk/AppFwk/cafHexInterpolator)

if(MSVC)
  target_compile_options(cafAnimControl PRIVATE /wd5054)
  target_compile_options(cafViewer PRIVATE /wd4996 /wd5054)
  target_compile_options(cafPdmCore PRIVATE /wd4996)
  target_compile_options(cafPdmUiCore PRIVATE /wd5054)
  target_compile_options(cafCommandFeatures PRIVATE /wd4996 /wd5054)
  target_compile_options(cafCommand PRIVATE /wd5054)
  target_compile_options(cafPdmXml PRIVATE /wd4996)
  target_compile_options(cafUserInterface PRIVATE /wd4996 /wd5054)
  target_compile_options(qwt PRIVATE /wd5055)
  target_compile_options(CommonCode PRIVATE /wd5054)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(cafCommandFeatures PRIVATE -Wno-deprecated-copy)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(
    CommonCode PRIVATE -Wno-deprecated-copy -Wno-undefined-var-template
  )
  target_compile_options(
    cafCommandFeatures PRIVATE -Wno-deprecated-copy -Wno-undefined-var-template
                               -Wno-deprecated-enum-enum-conversion
  )
  target_compile_options(
    cafVizExtensions PRIVATE -Wno-deprecated-copy -Wno-undefined-var-template
                             -Wno-invalid-source-encoding
  )
  target_compile_options(cafUserInterface PRIVATE -Wno-null-pointer-arithmetic)
endif()

list(
  APPEND
  APP_FWK_LIBRARIES
  cafPdmCore
  cafPdmUiCore
  cafPdmXml
  cafProjectDataModel
  cafUserInterface
  cafViewer
  cafAnimControl
  cafCommand
  cafPdmCvf
  cafTensor
  cafDataLoader
  CommonCode
  cafVizExtensions
  cafPdmScripting
  cafCommandFeatures
)

set_property(TARGET ${APP_FWK_LIBRARIES} PROPERTY FOLDER "AppFwk")

option(RESINSIGHT_INCLUDE_APPFWK_TESTS "Enable AppFwk Tests" OFF)
mark_as_advanced(FORCE RESINSIGHT_INCLUDE_APPFWK_TESTS)

if(RESINSIGHT_INCLUDE_APPFWK_TESTS)
  # Unit Tests
  add_subdirectory(Fwk/AppFwk/cafProjectDataModel/cafProjectDataModel_UnitTests)
  add_subdirectory(
    Fwk/AppFwk/cafProjectDataModel/cafPdmCore/cafPdmCore_UnitTests
  )
  add_subdirectory(Fwk/AppFwk/cafProjectDataModel/cafPdmXml/cafPdmXml_UnitTests)
  add_subdirectory(Fwk/AppFwk/cafPdmScripting/cafPdmScripting_UnitTests)
  add_subdirectory(Fwk/AppFwk/cafUserInterface/cafUserInterface_UnitTests)

  # Executables
  add_subdirectory(Fwk/AppFwk/cafTests/cafTestApplication)
  add_subdirectory(Fwk/AppFwk/cafTests/cafTestCvfApplication)

  list(
    APPEND
    APP_FWK_TEST_PROJECTS
    cafProjectDataModel_UnitTests
    cafPdmCore_UnitTests
    cafPdmXml_UnitTests
    cafPdmScripting_UnitTests
    cafUserInterface_UnitTests
    cafTestApplication
    cafTestCvfApplication
  )

  set_property(TARGET ${APP_FWK_TEST_PROJECTS} PROPERTY FOLDER "AppFwkTests")

endif()

# ##############################################################################
# Installation settings
# ##############################################################################

# Set the install folder to be the bin folder. TODO: Move Python scripts to a
# separate folder
set(RESINSIGHT_INSTALL_FOLDER "bin")

# override system install prefix if private installation chosen
option(RESINSIGHT_PRIVATE_INSTALL
       "Linux only: Install the resdata shared libraries along the executable"
       ON
)
mark_as_advanced(FORCE RESINSIGHT_PRIVATE_INSTALL)

if(RESINSIGHT_PRIVATE_INSTALL)

  # ############################################################################
  # ERT shared library files Install procedure will copy so-files from ERT into
  # same install folder as ResInsight
  # ############################################################################

  if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(ERT_SHARED_LIB_FILES
        ${CMAKE_BINARY_DIR}/ThirdParty/Ert/${CMAKE_INSTALL_LIBDIR}/libresdata.so
        ${CMAKE_BINARY_DIR}/ThirdParty/Ert/${CMAKE_INSTALL_LIBDIR}/libresdata.so.2
        ${CMAKE_BINARY_DIR}/ThirdParty/Ert/${CMAKE_INSTALL_LIBDIR}/libresdata.so.2.4
    )
    install(FILES ${ERT_SHARED_LIB_FILES}
            DESTINATION ${RESINSIGHT_INSTALL_FOLDER}
    )
  endif()

  if(APPLE)
    set(ERT_SHARED_LIB_FILES
        ${CMAKE_BINARY_DIR}/ThirdParty/Ert/${CMAKE_INSTALL_LIBDIR}/libresdata.dylib
        ${CMAKE_BINARY_DIR}/ThirdParty/Ert/${CMAKE_INSTALL_LIBDIR}/libresdata.2.dylib
        ${CMAKE_BINARY_DIR}/ThirdParty/Ert/${CMAKE_INSTALL_LIBDIR}/libresdata.2.4.dylib
    )
    install(
      FILES ${ERT_SHARED_LIB_FILES}
      DESTINATION ${RESINSIGHT_INSTALL_FOLDER}/ResInsight.app/Contents/MacOS
    )
  endif()

endif(RESINSIGHT_PRIVATE_INSTALL)

if(RESINSIGHT_HDF5_BUNDLE_LIBRARIES)
  if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # find all file names with text hdf5 use file globbing to also copy the
    # symlinks to make sure the dependencies from ResInsight runtime is correct
    foreach(FILE_TO_COPY ${HDF5_LIBRARIES})
      string(FIND ${FILE_TO_COPY} "hdf5" POS_IN_STRING)
      if(${POS_IN_STRING} GREATER -1)
        file(GLOB FILE_AND_SYMLINKS ${FILE_TO_COPY}*)
        install(FILES ${FILE_AND_SYMLINKS}
                DESTINATION ${RESINSIGHT_INSTALL_FOLDER}
        )
      endif()
    endforeach()
  endif()
endif(RESINSIGHT_HDF5_BUNDLE_LIBRARIES)

if(RESINSIGHT_BUNDLE_TESTMODELS)
  install(DIRECTORY ${CMAKE_SOURCE_DIR}/TestModels
          DESTINATION ${RESINSIGHT_INSTALL_FOLDER}
  )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(OPENVDS_SHARED_LIB_FILES
      ${RESINSIGHT_OPENVDS_API_DIR}/lib64/libopenvds.so
      ${RESINSIGHT_OPENVDS_API_DIR}/lib64/libopenvds.so.3
      ${RESINSIGHT_OPENVDS_API_DIR}/lib64/libopenvds.so.3.4.5
      ${RESINSIGHT_OPENVDS_API_DIR}/lib64/libopenvds-85fcb584.so.3.4.5
      ${RESINSIGHT_OPENVDS_API_DIR}/lib64/libsegyutils.so
      ${RESINSIGHT_OPENVDS_API_DIR}/lib64/libsegyutils.so.3
      ${RESINSIGHT_OPENVDS_API_DIR}/lib64/libsegyutils.so.3.4.5
  )
  install(FILES ${OPENVDS_SHARED_LIB_FILES}
          DESTINATION ${RESINSIGHT_INSTALL_FOLDER}
  )

  install(PROGRAMS ${RESINSIGHT_OPENVDS_API_DIR}/bin/SEGYImport
          DESTINATION ${RESINSIGHT_INSTALL_FOLDER}
  )
endif()

# ##############################################################################
# Application
# ##############################################################################
add_subdirectory(ApplicationLibCode)
add_subdirectory(ApplicationLibCode/Commands)
add_subdirectory(ApplicationLibCode/ResultStatisticsCache)
add_subdirectory(ApplicationLibCode/GeoMech/GeoMechDataModel)
add_subdirectory(ApplicationLibCode/GeoMech/GeoMechFileInterface)
if(RESINSIGHT_USE_ODB_API)
  add_subdirectory(ApplicationLibCode/GeoMech/OdbReader)
endif()

if(RESINSIGHT_ENABLE_GRPC)
  add_subdirectory(GrpcInterface)
endif()
add_subdirectory(ApplicationExeCode)

option(RESINSIGHT_USE_EXTERNAL_OCTAVE_PLUGINS "Use external Octave plugins" OFF)
if(RESINSIGHT_USE_EXTERNAL_OCTAVE_PLUGINS)

  if(MSVC)
    set(OCTAVE_PLUGINS_URL
        "https://github.com/CeetronSolutions/resinsight-dependencies/releases/latest/download/OctavePlugins-0.1.1-win64.zip"
    )
  else()
    set(OCTAVE_PLUGINS_URL
        "https://github.com/CeetronSolutions/resinsight-dependencies/releases/latest/download/OctavePlugins-0.1.1-Linux.tar.gz"
    )
  endif(MSVC)

  message(
    STATUS "Starting to fetch OctavePlugin binaries from ${OCTAVE_PLUGINS_URL}"
  )

  FetchContent_Declare(external-octave_plugins URL ${OCTAVE_PLUGINS_URL})

  message(STATUS "Completed fetching of OctavePlugin")

  FetchContent_MakeAvailable(external-octave_plugins)

  file(GLOB FILE_AND_SYMLINKS ${external-octave_plugins_SOURCE_DIR}/*.oct)
  install(
    FILES ${FILE_AND_SYMLINKS}
    DESTINATION ${RESINSIGHT_INSTALL_FOLDER}
    OPTIONAL
  )
else(RESINSIGHT_USE_EXTERNAL_OCTAVE_PLUGINS)
  if(OCTAVE_MKOCTFILE)
    message(STATUS "Adding OctavePlugin library")

    # The build of Octave Plugins is currently not working when Ninja is used as
    # build tool A separate build job is configured to build the Octave Plugins,
    # and the build is cached at GitHub
    #
    # add_subdirectory(Octave/OctavePlugin)
  else(OCTAVE_MKOCTFILE)
    message(
      STATUS "Could not find OCTAVE_MKOCTFILE, skipping OctavePlugin library"
    )
  endif(OCTAVE_MKOCTFILE)
endif(RESINSIGHT_USE_EXTERNAL_OCTAVE_PLUGINS)

add_subdirectory(ThirdParty/extract-projectfile-versions)
install(TARGETS extract-projectfile-versions
        DESTINATION ${RESINSIGHT_INSTALL_FOLDER}
)

# ##############################################################################
# Visual Studio : Create the ruleset file to be used by Static Code Analysis
# https://stackoverflow.com/questions/75031903/how-to-enable-static-analysis-with-custom-ruleset-in-msvc-via-cmakelists-txt
# ##############################################################################

option(RESINSIGHT_ENABLE_STATIC_ANALYSIS "Enable static analysis" OFF)
if(MSVC AND RESINSIGHT_ENABLE_STATIC_ANALYSIS)
  set(RULESET_FILE ${CMAKE_CURRENT_BINARY_DIR}/msvc-ruleset.props)
  configure_file(
    ${CMAKE_SOURCE_DIR}/scripts/static-code-analysis/msvc-property-sheet-template.props
    ${RULESET_FILE}
    @ONLY
  )

  set(TARGETS_FOR_STATIC_ANALYSIS
      ${APP_FWK_LIBRARIES}
      ${APP_FWK_TEST_PROJECTS}
      ApplicationLibCode
      Commands
      ResultStatisticsCache
      RigGeoMechDataModel
      RifGeoMechFileInterface
  )

  foreach(TARGET_PROJECT ${TARGETS_FOR_STATIC_ANALYSIS})
    set_property(
      TARGET ${TARGET_PROJECT} PROPERTY VS_USER_PROPS "${RULESET_FILE}"
    )
    target_compile_options(
      ${TARGET_PROJECT} PRIVATE /external:anglebrackets /analyze:external-
    )
  endforeach()
endif(MSVC AND RESINSIGHT_ENABLE_STATIC_ANALYSIS)

# ##############################################################################
# Code documentation using Doxygen
# ##############################################################################
option(RESINSIGHT_BUILD_DOCUMENTATION
       "Use Doxygen to create the HTML based API documentation" OFF
)
if(RESINSIGHT_BUILD_DOCUMENTATION)
  find_package(Doxygen)
  if(NOT DOXYGEN_FOUND)
    message(
      FATAL_ERROR
        "Doxygen is needed to build the documentation. Please install it correctly"
    )
  endif()
  # -- Configure the Template Doxyfile for our specific project
  configure_file(Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile @ONLY IMMEDIATE)
  # -- Add a custom target to run Doxygen when ever the project is built
  add_custom_target(
    Docs
    COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
    SOURCES ${PROJECT_BINARY_DIR}/Doxyfile
  )
  # IF you do NOT want the documentation to be generated EVERY time you build
  # the project then leave out the 'ALL' keyword from the above command.
endif(RESINSIGHT_BUILD_DOCUMENTATION)
