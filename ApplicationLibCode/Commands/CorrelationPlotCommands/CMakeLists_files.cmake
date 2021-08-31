set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicNewCorrelationPlotFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewCorrelationReportPlotFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewCorrelationMatrixPlotFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewParameterResultCrossPlotFeature.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicNewCorrelationPlotFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewCorrelationMatrixPlotFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewParameterResultCrossPlotFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewCorrelationReportPlotFeature.cpp
)

list(APPEND COMMAND_CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

source_group(
  "CommandFeature\\CorrelationPlotCommands"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake
)
