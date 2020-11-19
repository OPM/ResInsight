cmake_minimum_required(VERSION 3.12)

project(Commands)

# Unity Build
if(RESINSIGHT_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

set(COMMAND_REFERENCED_CMAKE_FILES
    CMakeLists_files.cmake
    AnalysisPlotCommands/CMakeLists_files.cmake
    ApplicationCommands/CMakeLists_files.cmake
    AnnotationCommands/CMakeLists_files.cmake
    CellFilterCommands/CMakeLists_files.cmake
    ColorLegendCommands/CMakeLists_files.cmake
    CompletionCommands/CMakeLists_files.cmake
    CompletionExportCommands/CMakeLists_files.cmake
    CorrelationPlotCommands/CMakeLists_files.cmake
    CrossSectionCommands/CMakeLists_files.cmake
    EclipseCommands/CMakeLists_files.cmake
    EclipseCommands/EclipseWell/CMakeLists_files.cmake
    ExportCommands/CMakeLists_files.cmake
    FlowCommands/CMakeLists_files.cmake
    GridCrossPlotCommands/CMakeLists_files.cmake
    HoloLensCommands/CMakeLists_files.cmake
    IntersectionBoxCommands/CMakeLists_files.cmake
    IntersectionViewCommands/CMakeLists_files.cmake
    MeasurementCommands/CMakeLists_files.cmake
    OctaveScriptCommands/CMakeLists_files.cmake
    OperationsUsingObjReferences/CMakeLists_files.cmake
    SummaryPlotCommands/CMakeLists_files.cmake
    SsiHubImportCommands/CMakeLists_files.cmake
    SurfaceCommands/CMakeLists_files.cmake
    ToggleCommands/CMakeLists_files.cmake
    ViewLink/CMakeLists_files.cmake
    WellLogCommands/CMakeLists_files.cmake
    WellPathCommands/CMakeLists_files.cmake
    PlotTemplateCommands/CMakeLists_files.cmake
    FractureCommands/CMakeLists_files.cmake)

# Include source file lists from *.cmake files
foreach(referencedfile ${COMMAND_REFERENCED_CMAKE_FILES})
  include(${referencedfile})
endforeach(referencedfile)

find_package(Eigen3 REQUIRED)

# Prefix files with COMMAND_ to avoid clash with application global lists
qt5_wrap_cpp(COMMAND_MOC_SOURCE_FILES ${COMMAND_QT_MOC_HEADERS})

add_library(
  ${PROJECT_NAME} OBJECT
  ${COMMAND_CODE_SOURCE_FILES} ${COMMAND_CODE_HEADER_FILES}
  ${COMMAND_MOC_SOURCE_FILES})

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/ThirdParty
         ${CMAKE_SOURCE_DIR}/ThirdParty/custom-opm-common/generated-opm-common
         ${CMAKE_SOURCE_DIR}/ThirdParty/custom-opm-common/opm-common)

set(LINK_LIBRARIES
    LibCore
    cafCommand
    cafPdmCvf
    cafPdmScripting
    cafTensor
    cafViewer
    cafVizExtensions
    ecl
    nightcharts
    qwt
    ${QT_LIBRARIES}
    Eigen3::Eigen)

target_link_libraries(${PROJECT_NAME} PRIVATE ${LINK_LIBRARIES})

source_group("" FILES ${PROJECT_FILES})
