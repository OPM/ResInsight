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

find_package(
  Qt6
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
             Sql
)
set(QT_LIBRARIES
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGL
    Qt6::Network
    Qt6::NetworkAuth
    Qt6::Widgets
    Qt6::Xml
    Qt6::Concurrent
    Qt6::PrintSupport
    Qt6::Svg
    Qt6::Sql
)
qt_standard_project_setup()

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
  Application/Tools/Cloud/CMakeLists_files.cmake
  Application/Tools/Summary/CMakeLists_files.cmake
  Application/Tools/KeyValueStore/CMakeLists_files.cmake
  Application/Tools/Ensemble/CMakeLists_files.cmake
  ReservoirDataModel/CMakeLists_files.cmake
  ReservoirDataModel/CMakeLists_filesNotToUnitTest.cmake
  ReservoirDataModel/Completions/CMakeLists_files.cmake
  ReservoirDataModel/SimulationFile/CMakeLists_files.cmake
  ReservoirDataModel/CompletionsMsw/CMakeLists_files.cmake
  ReservoirDataModel/ContourMap/CMakeLists_files.cmake
  ReservoirDataModel/ResultAccessors/CMakeLists_files.cmake
  ReservoirDataModel/ResultCalculators/CMakeLists_files.cmake
  ReservoirDataModel/Well/CMakeLists_files.cmake
  ReservoirDataModel/Surface/CMakeLists_files.cmake
  FileInterface/CMakeLists_files.cmake
  ProjectDataModel/CMakeLists_files.cmake
  ProjectDataModel/AnalysisPlots/CMakeLists_files.cmake
  ProjectDataModel/Appearance/CMakeLists_files.cmake
  ProjectDataModel/CorrelationPlots/CMakeLists_files.cmake
  ProjectDataModel/ContourMap/CMakeLists_files.cmake
  ProjectDataModel/Cloud/CMakeLists_files.cmake
  ProjectDataModel/EnsembleFileSet/CMakeLists_files.cmake
  ProjectDataModel/Faults/CMakeLists_files.cmake
  ProjectDataModel/GeoMech/CMakeLists_files.cmake
  ProjectDataModel/GridCrossPlots/CMakeLists_files.cmake
  ProjectDataModel/GridCrossPlots/CellFilters/CMakeLists_files.cmake
  ProjectDataModel/Jobs/CMakeLists_files.cmake
  ProjectDataModel/Summary/CMakeLists_files.cmake
  ProjectDataModel/Summary/Sumo/CMakeLists_files.cmake
  ProjectDataModel/Summary/Ensemble/CMakeLists_files.cmake
  ProjectDataModel/Flow/CMakeLists_files.cmake
  ProjectDataModel/Formations/CMakeLists_files.cmake
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
  ProjectDataModel/VerticalFlowPerformance/CMakeLists_files.cmake
  ProjectDataModel/QuickAccess/CMakeLists_files.cmake
  ProjectDataModel/Histogram/CMakeLists_files.cmake
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
if(RESINSIGHT_ENABLE_OPENVDS)
  message(STATUS "OpenVDS support enabled")
  add_definitions(-DUSE_OPENVDS)
  list(APPEND RI_PRIVATE_INCLUDES ${RESINSIGHT_OPENVDS_API_DIR}/include)

  list(APPEND CPP_SOURCES FileInterface/RifOpenVDSReader.cpp
       FileInterface/RifOpenVDSReader.h
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
endif()

#
# OpenTelemetry
#
set(OPENTELEMETRY_FILES
    Application/RiaPreferencesOpenTelemetry.h
    Application/RiaPreferencesOpenTelemetry.cpp
    Application/Tools/Telemetry/RiaOpenTelemetryManager.h
    Application/Tools/Telemetry/RiaOpenTelemetryManager.cpp
)
list(APPEND CPP_SOURCES ${OPENTELEMETRY_FILES})

qt_add_resources(QRC_FILES Application/Resources/ApplicationLibCode.qrc)

list(APPEND ALL_SOURCE_FILES ${CPP_SOURCES} ${MOC_SOURCE_FILES}
     ${FORM_FILES_CPP} ${QRC_FILES}
)

add_library(${PROJECT_NAME} OBJECT ${ALL_SOURCE_FILES})

option(RESINSIGHT_ENABLE_DYNAMIC_DEOPTIMALIZATION
       "Ensable Dynamic Deoptimalization" OFF
)
mark_as_advanced(FORCE RESINSIGHT_ENABLE_DYNAMIC_DEOPTIMALIZATION)
if(MSVC AND RESINSIGHT_ENABLE_DYNAMIC_DEOPTIMALIZATION)
  message("Dynamic Deoptimalization is enabled on : ${PROJECT_NAME}")
  target_compile_options(${PROJECT_NAME} PRIVATE /dynamicdeopt)
  target_link_options(${PROJECT_NAME} PRIVATE /DYNAMICDEOPT)
else()
  message("Dynamic Deoptimalization is disabled on : ${PROJECT_NAME}")
endif()

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
            -Wno-deprecated-declarations
  )
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

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  # According to ivarun rt is needed on OpenSuse, and Fedora. See:
  # https://github.com/OPM/ResInsight/pull/7
  list(APPEND EXTERNAL_LINK_LIBRARIES rt)

  # std::stacktrace requires stdc++exp. If a custom build of GCC stdc++exp is
  # required, define the search path here.
  set(RESINSIGHT_GCC_STDCPP_EXP_PATH
      ""
      CACHE PATH "Path to search for stdc++exp"
  )

  if(RESINSIGHT_GCC_STDCPP_EXP_PATH)
    find_library(
      STDCPP_EXP_LIBRARY
      NAMES stdc++exp
      HINTS ${RESINSIGHT_GCC_STDCPP_EXP_PATH}
    )
    if(STDCPP_EXP_LIBRARY)
      message(
        STATUS
          "stdc++exp library found at custom location: ${STDCPP_EXP_LIBRARY}"
      )
      list(APPEND EXTERNAL_LINK_LIBRARIES ${STDCPP_EXP_LIBRARY})
    endif()
  else()
    # stdc++exp is required for std::stacktrace
    message(STATUS "stdc++exp library found at default location")
    list(APPEND EXTERNAL_LINK_LIBRARIES stdc++exp)
  endif()
endif()

target_link_libraries(
  ApplicationLibCode PUBLIC ${LINK_LIBRARIES} ${EXTERNAL_LINK_LIBRARIES}
)
target_link_libraries(ApplicationLibCode PRIVATE ResInsightCommonSettings)

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
         ${CMAKE_CURRENT_SOURCE_DIR}/Application/Tools/KeyValueStore
         ${CMAKE_CURRENT_SOURCE_DIR}/Application/Tools/Telemetry
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
         ${CMAKE_CURRENT_SOURCE_DIR}/ReservoirDataModel/SimulationFile
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
    qrc_ApplicationLibCode.cpp
    # Exclude files including opm-common
    ProjectDataModel/RimVfpTableExtractor.cpp
    ProjectDataModel/RimVfpPlot.cpp
    # Exclude moc files as they cause template instantiation issues
    # https://cmake.org/cmake/help/latest/prop_tgt/AUTOGEN_BUILD_DIR.html
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_autogen/mocs_compilation.cpp
    ProjectDataModel/RiaOpmParserTools.cpp
    FileInterface/RifOsduWellPathReader.cpp
    FileInterface/RifOsduWellLogReader.cpp
    FileInterface/RifByteArrayArrowRandomAccessFile.cpp
    FileInterface/RifArrowTools.cpp
    FileInterface/RifReaderOpmRft.cpp
)

message("Files excluded from UNITY_BUILD : ${UNITY_EXCLUDE_FILES}")

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
