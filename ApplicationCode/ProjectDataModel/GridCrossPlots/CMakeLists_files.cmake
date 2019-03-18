
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RimGridCrossPlot.h
${CMAKE_CURRENT_LIST_DIR}/RimGridCrossPlotCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimGridCrossPlotCurve.h
${CMAKE_CURRENT_LIST_DIR}/RimGridCrossPlotCurveSet.h
${CMAKE_CURRENT_LIST_DIR}/RimSaturationPressurePlot.h
${CMAKE_CURRENT_LIST_DIR}/RimSaturationPressurePlotCollection.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RimGridCrossPlot.cpp
${CMAKE_CURRENT_LIST_DIR}/RimGridCrossPlotCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimGridCrossPlotCurve.cpp
${CMAKE_CURRENT_LIST_DIR}/RimGridCrossPlotCurveSet.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSaturationPressurePlot.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSaturationPressurePlotCollection.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "ProjectDataModel\\GridCrossPlots" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
