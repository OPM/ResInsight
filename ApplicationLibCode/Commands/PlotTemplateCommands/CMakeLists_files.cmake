set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicSavePlotTemplateFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicCreatePlotFromSelectionFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicSelectPlotTemplateUi.h
    ${CMAKE_CURRENT_LIST_DIR}/RicSummaryPlotTemplateTools.h
    ${CMAKE_CURRENT_LIST_DIR}/RicReloadPlotTemplatesFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicCreatePlotFromTemplateFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicCreatePlotFromTemplateByShortcutFeature.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicSavePlotTemplateFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCreatePlotFromSelectionFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSelectPlotTemplateUi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSummaryPlotTemplateTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicReloadPlotTemplatesFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCreatePlotFromTemplateFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCreatePlotFromTemplateByShortcutFeature.cpp
)

list(APPEND COMMAND_CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

list(APPEND COMMAND_QT_MOC_HEADERS)

source_group(
  "CommandFeature\\PlotTemplate"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake
)
