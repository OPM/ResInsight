
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicCreateGridCrossPlotFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicCreateGridCrossPlotCurveSetFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicSwapGridCrossPlotCurveSetAxesFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicPasteGridCrossPlotCurveSetFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicCreateSaturationPressurePlotsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicSaturationPressureUi.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicCreateGridCrossPlotFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicCreateGridCrossPlotCurveSetFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicSwapGridCrossPlotCurveSetAxesFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicPasteGridCrossPlotCurveSetFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicCreateSaturationPressurePlotsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicSaturationPressureUi.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\GridCrossPlot" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
