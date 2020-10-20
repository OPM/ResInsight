
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RimAbstractCorrelationPlot.h
${CMAKE_CURRENT_LIST_DIR}/RimCorrelationPlot.h
${CMAKE_CURRENT_LIST_DIR}/RimCorrelationMatrixPlot.h
${CMAKE_CURRENT_LIST_DIR}/RimParameterResultCrossPlot.h
${CMAKE_CURRENT_LIST_DIR}/RimCorrelationPlotCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimCorrelationReportPlot.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RimAbstractCorrelationPlot.cpp
${CMAKE_CURRENT_LIST_DIR}/RimCorrelationPlot.cpp
${CMAKE_CURRENT_LIST_DIR}/RimCorrelationMatrixPlot.cpp
${CMAKE_CURRENT_LIST_DIR}/RimParameterResultCrossPlot.cpp
${CMAKE_CURRENT_LIST_DIR}/RimCorrelationPlotCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimCorrelationReportPlot.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "ProjectDataModel\\CorrelationPlots" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
