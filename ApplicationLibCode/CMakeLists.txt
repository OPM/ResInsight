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
      "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-deprecated-declarations"
  )
endif()

# Open GL
find_package(OpenGL)

option(RESINSIGHT_TREAT_WARNINGS_AS_ERRORS
       "Treat warnings as errors (stops build)" OFF
)

find_package(Qt5 ${RI_QT_MINIMUM_VERSION} COMPONENTS Core)

if(Qt5Core_FOUND)
  find_package(
    Qt5 ${RI_QT_MINIMUM_VERSION}
    COMPONENTS Core
               Gui
               OpenGL
               Network
               NetworkAuth
               Widgets
               Xml
               Concurrent
               PrintSupport
               Svg
    OPTIONAL_COMPONENTS Charts
  )
  set(QT_LIBRARIES
      Qt5::Core
      Qt5::Gui
      Qt5::Network
      Qt5::NetworkAuth
      Qt5::OpenGL
      Qt5::Widgets
      Qt5::Xml
      Qt5::Concurrent
      Qt5::PrintSupport
      Qt5::Svg
  )
  if(Qt5Charts_FOUND)
    list(APPEND QT_LIBRARIES Qt5::Charts)
  endif(Qt5Charts_FOUND)
endif(Qt5Core_FOUND)

if(RESINSIGHT_GRPC_PYTHON_EXECUTABLE)
  set(Python3_EXECUTABLE ${RESINSIGHT_GRPC_PYTHON_EXECUTABLE})
  find_package(Python3)
  message(STATUS "Using Python Version: ${Python3_VERSION}")
endif()

# NB: The generated file is written to Cmake binary folder to avoid source tree
# pollution This folder is added to include_directories
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/Adm/RiaVersionInfo.h.cmake
  ${CMAKE_BINARY_DIR}/Generated/RiaVersionInfo.h
)

configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/RiuThemesDirectory.h.cmake
  ${CMAKE_BINARY_DIR}/Generated/RiuThemesDirectory.h
)
if(MSVC)
  # Avoid Qt warnings about macro redefinitions for math constants
  add_definitions(-D_USE_MATH_DEFINES)
endif()

# ##############################################################################
# Eigen
# ##############################################################################
if(RESINSIGHT_VCPKG_AUTO_INSTALL)
  vcpkg_install(eigen3)
endif()

find_package(Eigen3 REQUIRED)

# ##############################################################################
# Defining all the source (and header) files
# ##############################################################################

list(
  APPEND
  REFERENCED_CMAKE_FILES
  Application/CMakeLists_files.cmake
  Application/Tools/CMakeLists_files.cmake
  Application/Tools/WellPathTools/CMakeLists_files.cmake
  ReservoirDataModel/CMakeLists_files.cmake
  ReservoirDataModel/CMakeLists_filesNotToUnitTest.cmake
  ReservoirDataModel/Completions/CMakeLists_files.cmake
  ReservoirDataModel/ResultAccessors/CMakeLists_files.cmake
  ReservoirDataModel/ResultCalculators/CMakeLists_files.cmake
  FileInterface/CMakeLists_files.cmake
  ProjectDataModel/CMakeLists_files.cmake
  ProjectDataModel/AnalysisPlots/CMakeLists_files.cmake
  ProjectDataModel/CorrelationPlots/CMakeLists_files.cmake
  ProjectDataModel/Faults/CMakeLists_files.cmake
  ProjectDataModel/GeoMech/CMakeLists_files.cmake
  ProjectDataModel/GridCrossPlots/CMakeLists_files.cmake
  ProjectDataModel/GridCrossPlots/CellFilters/CMakeLists_files.cmake
  ProjectDataModel/Summary/CMakeLists_files.cmake
  ProjectDataModel/Flow/CMakeLists_files.cmake
  ProjectDataModel/Annotations/CMakeLists_files.cmake
  ProjectDataModel/Completions/CMakeLists_files.cmake
  ProjectDataModel/Measurement/CMakeLists_files.cmake
  ProjectDataModel/Parameters/CMakeLists_files.cmake
  ProjectDataModel/PlotTemplates/CMakeLists_files.cmake
  ProjectDataModel/StimPlanModel/CMakeLists_files.cmake
  ProjectDataModel/Streamlines/CMakeLists_files.cmake
  ProjectDataModel/Seismic/CMakeLists_files.cmake
  ProjectDataModel/Surfaces/CMakeLists_files.cmake
  ProjectDataModel/Intersections/CMakeLists_files.cmake
  ProjectDataModel/CellFilters/CMakeLists_files.cmake
  ProjectDataModel/ProcessControl/CMakeLists_files.cmake
  ProjectDataModel/Polygons/CMakeLists_files.cmake
  ProjectDataModel/WellLog/CMakeLists_files.cmake
  ProjectDataModel/WellMeasurement/CMakeLists_files.cmake
  ProjectDataModel/WellPath/CMakeLists_files.cmake
  ProjectDataModel/Tools/CMakeLists_files.cmake
  ProjectDataModelCommands/CMakeLists_files.cmake
  ProjectDataModelCommands/CommandRouter/CMakeLists_files.cmake
  GeoMech/GeoMechVisualization/CMakeLists_files.cmake
  ModelVisualization/CMakeLists_files.cmake
  ModelVisualization/Faults/CMakeLists_files.cmake
  ModelVisualization/GridBox/CMakeLists_files.cmake
  ModelVisualization/Intersections/CMakeLists_files.cmake
  ModelVisualization/Seismic/CMakeLists_files.cmake
  ModelVisualization/Streamlines/CMakeLists_files.cmake
  ModelVisualization/Surfaces/CMakeLists_files.cmake
  ModelVisualization/WindowEdgeAxesOverlayItem/CMakeLists_files.cmake
  UserInterface/CMakeLists_files.cmake
  UserInterface/AnalysisPlots/CMakeLists_files.cmake
  CommandFileInterface/CMakeLists_files.cmake
  CommandFileInterface/Core/CMakeLists_files.cmake
  SocketInterface/CMakeLists_files.cmake
)

# Include source file lists from *.cmake files
foreach(referencedfile ${REFERENCED_CMAKE_FILES})
  include(${referencedfile})
endforeach(referencedfile)

list(APPEND CPP_SOURCES ${CODE_SOURCE_FILES})

# ##############################################################################
# Sub-directory projects
# ##############################################################################

#
# Odb api
#
if(RESINSIGHT_USE_ODB_API)
  add_definitions(-DUSE_ODB_API)
endif()

#
# OpenVDS
#
list(APPEND RI_PRIVATE_INCLUDES ${RESINSIGHT_OPENVDS_API_DIR}/include)

#
# Configure include directories if opm-common.lib is downloaded and linked
# without building from source
#
if(NOT RESINSIGHT_BUILD_LIBS_FROM_SOURCE)
  list(APPEND RI_PUBLIC_INCLUDES
       ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/custom-opm-common/opm-common
  )
  list(
    APPEND
    RI_PRIVATE_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/custom-opm-common/generated-opm-common/include
  )
endif()

#
# HDF5
#
if(RESINSIGHT_FOUND_HDF5)

  set(HDF5_FILES
      FileInterface/RifHdf5Reader.h
      FileInterface/RifHdf5Reader.cpp
      FileInterface/RifHdf5SummaryReader.h
      FileInterface/RifHdf5SummaryReader.cpp
      FileInterface/RifOpmHdf5Summary.h
      FileInterface/RifOpmHdf5Summary.cpp
      FileInterface/RifHdf5Exporter.h
      FileInterface/RifHdf5Exporter.cpp
      FileInterface/RifHdf5SummaryExporter.h
      FileInterface/RifHdf5SummaryExporter.cpp
  )

  list(APPEND CPP_SOURCES ${HDF5_FILES})

  add_definitions(-DUSE_HDF5)
  add_definitions(-DH5_BUILT_AS_DYNAMIC_LIB)

  if(MSVC)
    list(APPEND RI_PRIVATE_INCLUDES ${RESINSIGHT_HDF5_DIR}/include)
  else()
    list(APPEND RI_PRIVATE_INCLUDES ${HDF5_INCLUDE_DIRS})
  endif() # MSVC

  source_group("FileInterface\\HDF5" FILES ${HDF5_FILES})

endif()

# ##############################################################################
# Qt specifics: Moc, ui, resources
# ##############################################################################

set(QT_MOC_HEADERS
    ${QT_MOC_HEADERS} ProjectDataModel/RimMimeData.h
    ProjectDataModel/Intersections/RimBoxIntersection.h
    SocketInterface/RiaSocketServer.h
)

qt5_wrap_cpp(MOC_SOURCE_FILES ${QT_MOC_HEADERS})
qt5_wrap_ui(FORM_FILES_CPP ${QT_UI_FILES})

# ##############################################################################
# Create source groups - see also included CMakeLists_files.cmake
# ##############################################################################
source_group("ModelVisualization" FILES ${MODEL_VISUALIZATION_FILES})

list(APPEND ALL_SOURCE_FILES ${CPP_SOURCES} ${MOC_SOURCE_FILES}
     ${FORM_FILES_CPP}
)

add_library(${PROJECT_NAME} OBJECT ${ALL_SOURCE_FILES})

option(RESINSIGHT_ENABLE_PRECOMPILED_HEADERS "Use Precompiled Headers" OFF)
mark_as_advanced(FORCE RESINSIGHT_ENABLE_PRECOMPILED_HEADERS)
if(RESINSIGHT_ENABLE_PRECOMPILED_HEADERS)
  message("Precompiled Headers is enabled on : ${PROJECT_NAME}")
  target_precompile_headers(ApplicationLibCode PRIVATE pch.h)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(
    ApplicationLibCode PRIVATE -Wall -Wno-reorder -Wno-parentheses -Wno-switch
  )
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

  target_compile_options(
    ApplicationLibCode
    PRIVATE -Wall
            -Wno-reorder
            -Wno-switch
            -Wno-undefined-var-template
            -Wno-delete-abstract-non-virtual-dtor
            -Wno-call-to-pure-virtual-from-ctor-dtor
            -Wno-delete-non-abstract-non-virtual-dtor
            -Wno-ambiguous-reversed-operator
  )
endif()

if(RESINSIGHT_TREAT_WARNINGS_AS_ERRORS)
  if(MSVC)
    target_compile_options(ApplicationLibCode PRIVATE /WX)
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_compile_options(ApplicationLibCode PRIVATE -Werror)
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

  set(BUILD_FLAGS_FOR_MSVC "/W3 /wd4190 /wd4100 /wd4127")

  message(STATUS "CMAKE_CXX_COMPILER_VERSION ${CMAKE_CXX_COMPILER_VERSION}")

  if(CMAKE_CXX_COMPILER_VERSION LESS_EQUAL 19.14)
    # The following warning is generated over 800 times from a qwt header only
    # using VS2015 Disabling temporarily warning C4505 'function' : unreferenced
    # local function has been removed
    set(BUILD_FLAGS_FOR_MSVC "${BUILD_FLAGS_FOR_MSVC} /wd4505")
  endif()

  if(CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 19.38)
    # https://github.com/OPM/ResInsight/issues/10844
    set(BUILD_FLAGS_FOR_MSVC "${BUILD_FLAGS_FOR_MSVC} /wd4996")
  endif()

  message(STATUS "BUILD_FLAGS_FOR_MSVC ${BUILD_FLAGS_FOR_MSVC}")
  set_target_properties(
    ${PROJECT_NAME} PROPERTIES COMPILE_FLAGS ${BUILD_FLAGS_FOR_MSVC}
  )

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
    ${RI_LIBRARIES}
)

# According to ivarun this is needed on OpenSuse, and Fedora. See:
# https://github.com/OPM/ResInsight/pull/7
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  list(APPEND EXTERNAL_LINK_LIBRARIES rt)
endif()

target_link_libraries(
  ${PROJECT_NAME} PUBLIC ${LINK_LIBRARIES} ${EXTERNAL_LINK_LIBRARIES}
)

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Commands
          ${CMAKE_CURRENT_SOURCE_DIR}/Commands/EclipseCommands
          ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModelCommands/CommandRouter
          ${RI_PRIVATE_INCLUDES}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/Adm
         ${CMAKE_CURRENT_SOURCE_DIR}/Application
         ${CMAKE_CURRENT_SOURCE_DIR}/Application/Tools
         ${CMAKE_CURRENT_SOURCE_DIR}/Application/Tools/WellPathTools
         ${CMAKE_CURRENT_SOURCE_DIR}/CommandFileInterface
         ${CMAKE_CURRENT_SOURCE_DIR}/CommandFileInterface/Core
         ${CMAKE_CURRENT_SOURCE_DIR}/FileInterface
         ${CMAKE_CURRENT_SOURCE_DIR}/GeoMech
         ${CMAKE_CURRENT_SOURCE_DIR}/ModelVisualization
         ${CMAKE_CURRENT_SOURCE_DIR}/ModelVisualization/Faults
         ${CMAKE_CURRENT_SOURCE_DIR}/ModelVisualization/GridBox
         ${CMAKE_CURRENT_SOURCE_DIR}/ModelVisualization/Intersections
         ${CMAKE_CURRENT_SOURCE_DIR}/ModelVisualization/Seismic
         ${CMAKE_CURRENT_SOURCE_DIR}/ModelVisualization/Streamlines
         ${CMAKE_CURRENT_SOURCE_DIR}/ModelVisualization/Surfaces
         ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel
         ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/AnalysisPlots
         ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/CorrelationPlots
         ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Annotations
         ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Completions
         ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Faults
         ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Flow
         ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/GridCrossPlots
         ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/GeoMech
         ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Parameters
         ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Measurement
         ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/StimPlanModel
         ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Summary
         ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Surfaces
         ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Intersections
         ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Streamlines
         ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/Seismic
         ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/CellFilters
         ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/ProcessControl
         ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/WellLog
         ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/WellMeasurement
         ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModel/WellPath
         ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModelCommands
         ${CMAKE_CURRENT_SOURCE_DIR}/ProjectDataModelCommands/CommandRouter
         ${CMAKE_CURRENT_SOURCE_DIR}/ReservoirDataModel
         ${CMAKE_CURRENT_SOURCE_DIR}/ReservoirDataModel/Completions
         ${CMAKE_CURRENT_SOURCE_DIR}/ReservoirDataModel/ResultAccessors
         ${CMAKE_CURRENT_SOURCE_DIR}/ReservoirDataModel/ResultCalculators
         ${CMAKE_CURRENT_SOURCE_DIR}/ResultStatisticsCache
         ${CMAKE_CURRENT_SOURCE_DIR}/SocketInterface
         ${CMAKE_CURRENT_SOURCE_DIR}/UserInterface
         ${CMAKE_CURRENT_SOURCE_DIR}/UserInterface/AnalysisPlots
         ${CMAKE_CURRENT_SOURCE_DIR}/GeoMech/GeoMechDataModel
         ${CMAKE_CURRENT_SOURCE_DIR}/GeoMech/GeoMechVisualization
         ${CMAKE_CURRENT_SOURCE_DIR}/GeoMech/GeoMechFileInterface
         ${CMAKE_CURRENT_SOURCE_DIR}/GeoMech/OdbReader
         ${CMAKE_CURRENT_SOURCE_DIR}/Measurement
         ${ResInsight_SOURCE_DIR}/ThirdParty
         ${CMAKE_BINARY_DIR}/Generated
         ${RI_PUBLIC_INCLUDES}
)

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
    ProjectDataModel/RimVfpPlot.cpp
)

if(RESINSIGHT_ENABLE_UNITY_BUILD)
  foreach(fileToExclude ${UNITY_EXCLUDE_FILES})
    set_source_files_properties(
      ${fileToExclude} PROPERTIES SKIP_UNITY_BUILD_INCLUSION TRUE
    )
  endforeach(fileToExclude)
endif()

# ##############################################################################
# Unit tests
# ##############################################################################

option(RESINSIGHT_INCLUDE_APPLICATION_UNIT_TESTS
       "Include ApplicationLibCode Unit Tests" OFF
)
mark_as_advanced(FORCE RESINSIGHT_INCLUDE_APPLICATION_UNIT_TESTS)
if(RESINSIGHT_INCLUDE_APPLICATION_UNIT_TESTS)
  enable_testing()
  add_subdirectory(UnitTests)
endif()
