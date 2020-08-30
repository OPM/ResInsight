
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicCreateGridCrossPlotFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicCreateGridCrossPlotDataSetFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicSwapGridCrossPlotDataSetAxesFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicPasteGridCrossPlotDataSetFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicCreateSaturationPressurePlotsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicSaturationPressureUi.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicCreateGridCrossPlotFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicCreateGridCrossPlotDataSetFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicSwapGridCrossPlotDataSetAxesFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicPasteGridCrossPlotDataSetFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicCreateSaturationPressurePlotsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicSaturationPressureUi.cpp
)

list(APPEND COMMAND_CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND COMMAND_CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\GridCrossPlot" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
