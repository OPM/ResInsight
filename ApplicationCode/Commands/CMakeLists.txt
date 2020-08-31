cmake_minimum_required (VERSION 2.8.12)

project (Commands)

# Unity Build
if (RESINSIGHT_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

set( COMMAND_REFERENCED_CMAKE_FILES
    CMakeLists_files.cmake
    AnalysisPlotCommands/CMakeLists_files.cmake
    ApplicationCommands/CMakeLists_files.cmake
    AnnotationCommands/CMakeLists_files.cmake
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
    FractureCommands/CMakeLists_files.cmake
)

# Include source file lists from *.cmake files
foreach (referencedfile ${COMMAND_REFERENCED_CMAKE_FILES})
    include (${referencedfile})
endforeach (referencedfile)


# Prefix files with COMMAND_ to avoid clash with application global lists
qt5_wrap_cpp(COMMAND_MOC_SOURCE_FILES ${COMMAND_QT_MOC_HEADERS} )

add_library( ${PROJECT_NAME} OBJECT
    ${COMMAND_CODE_SOURCE_FILES}
    ${COMMAND_CODE_HEADER_FILES}
    ${COMMAND_MOC_SOURCE_FILES}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
  $<TARGET_PROPERTY:LibCore,INCLUDE_DIRECTORIES>        # Needed for cmake version < 3.12. Remove when we can use target_link_libraries
  $<TARGET_PROPERTY:cafCommand,INCLUDE_DIRECTORIES>     # Needed for cmake version < 3.12. Remove when we can use target_link_libraries
  $<TARGET_PROPERTY:cafPdmCvf,INCLUDE_DIRECTORIES>      # Needed for cmake version < 3.12. Remove when we can use target_link_libraries
  $<TARGET_PROPERTY:cafTensor,INCLUDE_DIRECTORIES>      # Needed for cmake version < 3.12. Remove when we can use target_link_libraries
  $<TARGET_PROPERTY:cafViewer,INCLUDE_DIRECTORIES>      # Needed for cmake version < 3.12. Remove when we can use target_link_libraries
  $<TARGET_PROPERTY:cafVizExtensions,INCLUDE_DIRECTORIES>       # Needed for cmake version < 3.12. Remove when we can use target_link_libraries
  $<TARGET_PROPERTY:ecl,INCLUDE_DIRECTORIES>            # Needed for cmake version < 3.12. Remove when we can use target_link_libraries
  $<TARGET_PROPERTY:nightcharts,INCLUDE_DIRECTORIES>    # Needed for cmake version < 3.12. Remove when we can use target_link_libraries
  $<TARGET_PROPERTY:qwt,INCLUDE_DIRECTORIES>            # Needed for cmake version < 3.12. Remove when we can use target_link_libraries
  ${Qt5Network_INCLUDE_DIRS}                           # Needed for cmake version < 3.12. Remove when we can use target_link_libraries
)

#  Before cmake 3.12 OBJECT libraries could not use the target_link_libraries command, 
#  So we need to set the POSITION_INDEPENDENT_CODE option  manually
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

#  Use this code when 3.12 can be used as minimum version of cmake
#set( LINK_LIBRARIES
#  LibCore 
#  cafCommand
#  cafPdmCvf
#  cafTensor
#  cafViewer
#  cafVizExtensions
#  ecl
#  nightcharts
#  qwt

#  ${QT_LIBRARIES}
#)

#target_link_libraries( ${PROJECT_NAME}
#  ${LINK_LIBRARIES}
#)

source_group("" FILES ${PROJECT_FILES})
