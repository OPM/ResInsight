cmake_minimum_required(VERSION 3.12)

project(ApplicationLibCode)

# set packaging dir
if(NOT CPACK_PACKAGE_DIRECTORY)
  set(CPACK_PACKAGE_DIRECTORY ${CMAKE_BINARY_DIR}/packages)
endif()

if(RESINSIGHT_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-deprecated-declarations")
endif()

# Open GL
find_package(OpenGL)

option(RESINSIGHT_TREAT_WARNINGS_AS_ERRORS
       "Treat warnings as errors (stops build)" OFF)

find_package(
  Qt5
  COMPONENTS Core
  QUIET)

if(Qt5Core_FOUND)
  find_package(
    Qt5
    COMPONENTS Core
               Gui
               OpenGL
               Network
               Script
               Widgets
               Xml
               Concurrent
               PrintSupport
               Svg
    OPTIONAL_COMPONENTS Charts)
  set(QT_LIBRARIES
      Qt5::Core
      Qt5::Gui
      Qt5::Network
      Qt5::OpenGL
      Qt5::Script
      Qt5::Widgets
      Qt5::Xml
      Qt5::Concurrent
      Qt5::PrintSupport
      Qt5::Svg)
  if(Qt5Charts_FOUND)
    list(APPEND QT_LIBRARIES Qt5::Charts)
  endif(Qt5Charts_FOUND)
endif(Qt5Core_FOUND)

# NB: The generated file is written to Cmake binary folder to avoid source tree
# pollution This folder is added to include_directories
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Adm/RiaVersionInfo.h.cmake
               ${CMAKE_BINARY_DIR}/Generated/RiaVersionInfo.h)

configure_file(${CMAKE_CURRENT_LIST_DIR}/RiuThemesDirectory.h.cmake
               ${CMAKE_BINARY_DIR}/Generated/RiuThemesDirectory.h)
if(MSVC)
  # Avoid Qt warnings about macro redefinitions for math constants
  add_definitions(-D_USE_MATH_DEFINES)
endif()

include_directories(
  # required for compilation of gtest, should be refactored
  ${ResInsight_SOURCE_DIR}/ThirdParty
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/Adm
  ${CMAKE_CURRENT_SOURCE_DIR}/Application
  ${CMAKE_CURRENT_SOURCE_DIR}/Application/Tools
  ${CMAKE_CURRENT_SOURCE_DIR}/Application/Tools/WellPathTools
  ${CMAKE_CURRENT_SOURCE_DIR}/Commands
  ${CMAKE_CURRENT_SOURCE_DIR}/Commands/EclipseCommands
  ${CMAKE_CURRENT_SOURCE_DIR}/FileInterface
  ${CMAKE_CURRENT_SOURCE_DIR}/SocketInterface
  ${CMAKE_CURRENT_SOURCE_DIR}/Measurement
  ${CMAKE_CURRENT_SOURCE_DIR}/ModelVisualization
  ${CMAKE_CURRENT_SOURCE_DIR}/ModelVisualization/GridBox
  ${CMAKE_CURRENT_SOURCE_DIR}/ModelVisualization/Intersections
  ${CMAKE_CURRENT_SOURCE_DIR}/ModelVisualization/Surfaces
  ${CMAKE_CURRENT_SOURCE_DIR}/UserInterface
  ${CMAKE_CURRENT_SOURCE_DIR}/UserInterface/AnalysisPlots
  ${CMAKE_CURRENT_SOURCE_DIR}/CommandFileInterface
  ${CMAKE_CURRENT_SOURCE_DIR}/CommandFileInterface/Core
  ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel
  ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/AnalysisPlots
  ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/CorrelationPlots
  ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Annotations
  ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Completions
  ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Flow
  ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/GridCrossPlots
  ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Measurement
  ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/StimPlanModel
  ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Summary
  ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Surfaces
  ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/CellFilters
  ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModelCommands
  ${CMAKE_CURRENT_SOURCE_DIR}/ResultStatisticsCache
  ${CMAKE_CURRENT_SOURCE_DIR}/ReservoirDataModel
  ${CMAKE_CURRENT_SOURCE_DIR}/ReservoirDataModel/Completions
  ${CMAKE_CURRENT_SOURCE_DIR}/GeoMech/OdbReader
  ${CMAKE_CURRENT_SOURCE_DIR}/GeoMech/GeoMechDataModel
  ${CMAKE_CURRENT_SOURCE_DIR}/GeoMech/GeoMechVisualization
  ${CMAKE_BINARY_DIR}/Generated
  ${CMAKE_CURRENT_BINARY_DIR})

# ##############################################################################
# Eigen
# ##############################################################################
if(VCPKG_AUTO_INSTALL)
  vcpkg_install(eigen3)
endif()

find_package(Eigen3 REQUIRED)

# ##############################################################################
# Defining all the source (and header) files
# ##############################################################################

# Use all h files in the subdirectories to make them available in the project
file(GLOB_RECURSE HEADER_FILES *.h)

set(SOCKET_INTERFACE_FILES
    SocketInterface/RiaSocketServer.cpp
    SocketInterface/RiaProjectInfoCommands.cpp
    SocketInterface/RiaCaseInfoCommands.cpp
    SocketInterface/RiaGeometryCommands.cpp
    SocketInterface/RiaNNCCommands.cpp
    SocketInterface/RiaPropertyDataCommands.cpp
    SocketInterface/RiaWellDataCommands.cpp
    SocketInterface/RiaSocketTools.cpp
    SocketInterface/RiaSocketDataTransfer.cpp)

list(APPEND CPP_SOURCES ${SOCKET_INTERFACE_FILES} ${UNIT_TEST_FILES})

list(
  APPEND
  REFERENCED_CMAKE_FILES
  Application/CMakeLists_files.cmake
  Application/Tools/CMakeLists_files.cmake
  Application/Tools/WellPathTools/CMakeLists_files.cmake
  ReservoirDataModel/CMakeLists_files.cmake
  ReservoirDataModel/CMakeLists_filesNotToUnitTest.cmake
  ReservoirDataModel/Completions/CMakeLists_files.cmake
  FileInterface/CMakeLists_files.cmake
  ProjectDataModel/CMakeLists_files.cmake
  ProjectDataModel/AnalysisPlots/CMakeLists_files.cmake
  ProjectDataModel/CorrelationPlots/CMakeLists_files.cmake
  ProjectDataModel/GridCrossPlots/CMakeLists_files.cmake
  ProjectDataModel/GridCrossPlots/CellFilters/CMakeLists_files.cmake
  ProjectDataModel/Summary/CMakeLists_files.cmake
  ProjectDataModel/Flow/CMakeLists_files.cmake
  ProjectDataModel/Annotations/CMakeLists_files.cmake
  ProjectDataModel/Completions/CMakeLists_files.cmake
  ProjectDataModel/Measurement/CMakeLists_files.cmake
  ProjectDataModel/PlotTemplates/CMakeLists_files.cmake
  ProjectDataModel/StimPlanModel/CMakeLists_files.cmake
  ProjectDataModel/Surfaces/CMakeLists_files.cmake
  ProjectDataModel/CellFilters/CMakeLists_files.cmake
  ProjectDataModelCommands/CMakeLists_files.cmake
  GeoMech/GeoMechVisualization/CMakeLists_files.cmake
  ModelVisualization/CMakeLists_files.cmake
  ModelVisualization/GridBox/CMakeLists_files.cmake
  ModelVisualization/Intersections/CMakeLists_files.cmake
  ModelVisualization/Surfaces/CMakeLists_files.cmake
  ModelVisualization/WindowEdgeAxesOverlayItem/CMakeLists_files.cmake
  UserInterface/CMakeLists_files.cmake
  UserInterface/AnalysisPlots/CMakeLists_files.cmake
  CommandFileInterface/CMakeLists_files.cmake
  CommandFileInterface/Core/CMakeLists_files.cmake)

option(RESINSIGHT_INCLUDE_APPLICATION_UNIT_TESTS
       "Include ApplicationCode Unit Tests" OFF)
mark_as_advanced(FORCE RESINSIGHT_INCLUDE_APPLICATION_UNIT_TESTS)
if(RESINSIGHT_INCLUDE_APPLICATION_UNIT_TESTS)
  add_definitions(-DUSE_UNIT_TESTS)

  list(APPEND REFERENCED_CMAKE_FILES UnitTests/CMakeLists_files.cmake)

  list(APPEND CPP_SOURCES
       ${ResInsight_SOURCE_DIR}/ThirdParty/gtest/gtest-all.cc)

endif()

# Include source file lists from *.cmake files
foreach(referencedfile ${REFERENCED_CMAKE_FILES})
  include(${referencedfile})
endforeach(referencedfile)

list(APPEND CPP_SOURCES ${CODE_SOURCE_FILES})

# ##############################################################################
# Sub-directory projects
# ##############################################################################

add_subdirectory(ResultStatisticsCache)
add_subdirectory(Commands)

set(RI_LIBRARIES ResultStatisticsCache Commands)

#
# Odb api
#
set(RESINSIGHT_ODB_API_DIR
    ""
    CACHE
      PATH
      "Optional path to the ABAQUS ODB API from Simulia. Needed for support of geomechanical models"
)
if(NOT ${RESINSIGHT_ODB_API_DIR} EQUAL "")
  add_definitions(-DUSE_ODB_API)
  add_subdirectory(GeoMech/OdbReader)
  set(RESINSIGHT_USE_ODB_API 1)
  message(STATUS "Using ODB-Api from : ${RESINSIGHT_ODB_API_DIR}")
endif()

add_subdirectory(GeoMech/GeoMechDataModel)
list(APPEND RI_LIBRARIES RigGeoMechDataModel)

if(RESINSIGHT_USE_ODB_API)
  list(APPEND RI_LIBRARIES RifOdbReader)
endif()

#
# HDF5
#
if(RESINSIGHT_FOUND_HDF5)
  list(APPEND CPP_SOURCES FileInterface/RifHdf5Reader.h
       FileInterface/RifHdf5Reader.cpp)

  add_definitions(-DUSE_HDF5)

  if(MSVC)
    add_definitions(-DH5_BUILT_AS_DYNAMIC_LIB)

    include_directories(${RESINSIGHT_HDF5_DIR}/include)
  else()
    add_definitions(-DH5_BUILT_AS_DYNAMIC_LIB)
    add_definitions(${HDF5_DEFINITIONS})

    include_directories(${HDF5_INCLUDE_DIRS})
  endif() # MSVC

  source_group("FileInterface" FILES FileInterface/RifHdf5Reader.h
                                     FileInterface/RifHdf5Reader.cpp)
endif()

# ##############################################################################
# Qt specifics: Moc, ui, resources
# ##############################################################################

set(QT_MOC_HEADERS
    ${QT_MOC_HEADERS} ProjectDataModel/RimMimeData.h
    ProjectDataModel/RimBoxIntersection.h SocketInterface/RiaSocketServer.h)

qt5_wrap_cpp(MOC_SOURCE_FILES ${QT_MOC_HEADERS})
qt5_wrap_ui(FORM_FILES_CPP ${QT_UI_FILES})

# ##############################################################################
# Create source groups - see also included CMakeLists_files.cmake
# ##############################################################################
source_group("ModelVisualization" FILES ${MODEL_VISUALIZATION_FILES})
source_group("SocketInterface" FILES ${SOCKET_INTERFACE_FILES})
source_group("UnitTests" FILES ${UNIT_TEST_FILES})

list(
  APPEND
  ALL_SOURCE_FILES
  ${CPP_SOURCES}
  ${MOC_SOURCE_FILES}
  ${FORM_FILES_CPP}
  ${HEADER_FILES}
  ${REFERENCED_CMAKE_FILES}
  ../ResInsightVersion.cmake
  .clang-format
  .clang-tidy
  Adm/RiaVersionInfo.h.cmake)

add_library(${PROJECT_NAME} OBJECT ${ALL_SOURCE_FILES})

option(RESINSIGHT_ENABLE_PRECOMPILED_HEADERS "Use Precompiled Headers" OFF)
mark_as_advanced(FORCE RESINSIGHT_ENABLE_PRECOMPILED_HEADERS)
if(RESINSIGHT_ENABLE_PRECOMPILED_HEADERS)
  message("Precompiled Headers is enabled on : ${PROJECT_NAME}")
  target_precompile_headers(ResInsight PRIVATE pch.h)

  set_source_files_properties(
    ${ResInsight_SOURCE_DIR}/ThirdParty/gtest/gtest-all.cc
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
      COMPILE_FLAGS
      "-Wall -Wno-unused-parameter -Wno-reorder -Wno-parentheses -Wno-switch")
  # Treat warnings as errors if asked to do so
  if(RESINSIGHT_TREAT_WARNINGS_AS_ERRORS)
    set_target_properties(
      ${PROJECT_NAME}
      PROPERTIES
        COMPILE_FLAGS
        "-Wall -Wno-unused-parameter -Wno-reorder -Wno-parentheses -Wno-switch -Werror"
    )
  endif()

  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set_target_properties(
      ${PROJECT_NAME}
      PROPERTIES
        COMPILE_FLAGS
        "-Wall -Wno-unused-parameter -Wno-reorder -Wno-parentheses -Wno-switch -Wno-delete-abstract-non-virtual-dtor -Wno-undefined-var-template -Wno-invalid-source-encoding -Wno-enum-compare -Wno-call-to-pure-virtual-from-ctor-dtor -Wno-unused-variable -Wno-unused-private-field -Wno-unused-lambda-capture -Wno-delete-non-abstract-non-virtual-dtor -Wno-braced-scalar-init -Wno-tautological-constant-out-of-range-compare"
    )
  endif()
endif()

if(MSVC)
  # The following warnings are supposed to be used in ResInsight, but
  # temporarily disabled to avoid too much noise warning C4245: 'return':
  # conversion from 'int' to 'size_t', signed/unsigned mismatch warning C4005:
  # Macro redefinition for math constants (M_PI, M_SQRT2 etc)

  # If possible, the following command is supposed to be the final target
  # set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W3 /wd4190
  # /wd4100 /wd4127")

  set(BUILD_FLAGS_FOR_MSVC "/wd4190 /wd4100 /wd4127 /wd4245 /wd4005")

  if(Qt5Core_VERSION_STRING GREATER_EQUAL 5.10)
    # Disable warning for deprecated functions in newer versions of Qt
    # https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-3-c4996?view=msvc-160
    set(BUILD_FLAGS_FOR_MSVC "${BUILD_FLAGS_FOR_MSVC} /wd4996")
  endif()

  if(CMAKE_CXX_COMPILER_VERSION LESS_EQUAL 19.14)
    # The following warning is generated over 800 times from a qwt header only
    # using VS2015 Disabling temporarily warning C4505 'function' : unreferenced
    # local function has been removed
    set(BUILD_FLAGS_FOR_MSVC "${BUILD_FLAGS_FOR_MSVC} /wd4505")
  endif()

  message(STATUS "BUILD_FLAGS_FOR_MSVC ${BUILD_FLAGS_FOR_MSVC}")
  set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS
                                                   ${BUILD_FLAGS_FOR_MSVC})

endif()

# ##############################################################################
# Set up libraries and dependent projects to link with
# ##############################################################################

set(LINK_LIBRARIES
    Eigen3::Eigen
    ${THIRD_PARTY_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${QT_LIBRARIES}
    ${OPM_LIBRARIES}
    ${APP_FWK_LIBRARIES}
    ${VIZ_FWK_LIBRARIES}
    ${RI_LIBRARIES})

# According to ivarun this is needed on OpenSuse, and Fedora. See:
# https://github.com/OPM/ResInsight/pull/7
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  list(APPEND EXTERNAL_LINK_LIBRARIES rt)
endif()

target_link_libraries(${PROJECT_NAME} ${LINK_LIBRARIES}
                      ${EXTERNAL_LINK_LIBRARIES})

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE ${CMAKE_BINARY_DIR}/Generated
  PUBLIC ${CMAKE_SOURCE_DIR}/ApplicationLibCode/Application
         ${CMAKE_SOURCE_DIR}/ApplicationLibCode/Application/Tools
         ${CMAKE_SOURCE_DIR}/ApplicationLibCode/CommandFileInterface
         ${CMAKE_SOURCE_DIR}/ApplicationLibCode/CommandFileInterface/Core
         ${CMAKE_SOURCE_DIR}/ApplicationLibCode/FileInterface
         ${CMAKE_SOURCE_DIR}/ApplicationLibCode/GeoMech
         ${CMAKE_SOURCE_DIR}/ApplicationLibCode/ModelVisualization
         ${CMAKE_SOURCE_DIR}/ApplicationLibCode/ProjectDataModel
         ${CMAKE_SOURCE_DIR}/ApplicationLibCode/ProjectDataModel/Flow
         ${CMAKE_SOURCE_DIR}/ApplicationLibCode/ReservoirDataModel
         ${CMAKE_SOURCE_DIR}/ApplicationLibCode/SocketInterface
         ${CMAKE_SOURCE_DIR}/ApplicationLibCode/UserInterface)

# ##############################################################################
# Unity builds
# ##############################################################################

set(UNITY_EXCLUDE_FILES
    # forever is used as variable name, and this symbol is defined by Qt and
    # used in precompiled headers
    ${ResInsight_SOURCE_DIR}/ThirdParty/gtest/gtest-all.cc
    qrc_cafAnimControl.cpp
    qrc_ResInsight.cpp
    qrc_cafCommandFeatures.cpp
    # Exclude files including opm-common
    ProjectDataModel/RimVfpTableExtractor.cpp
    ProjectDataModel/RimVfpPlot.cpp)

if(RESINSIGHT_ENABLE_UNITY_BUILD)
  foreach(fileToExclude ${UNITY_EXCLUDE_FILES})
    set_source_files_properties(${fileToExclude}
                                PROPERTIES SKIP_UNITY_BUILD_INCLUSION TRUE)
  endforeach(fileToExclude)
endif()
