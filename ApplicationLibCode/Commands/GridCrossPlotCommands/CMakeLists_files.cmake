set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateGridCrossPlotFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateGridCrossPlotDataSetFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSwapGridCrossPlotDataSetAxesFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicPasteGridCrossPlotDataSetFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateSaturationPressurePlotsFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSaturationPressureUi.cpp
)

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
