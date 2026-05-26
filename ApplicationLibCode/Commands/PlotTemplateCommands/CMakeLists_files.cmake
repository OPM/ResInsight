set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicSelectPlotTemplateUi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicReloadPlotTemplatesFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCreatePlotFromTemplateByShortcutFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSaveMultiPlotTemplateFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSaveMultiPlotTemplateFeatureSettings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateMultiPlotFromSelectionFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicRenamePlotTemplateFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicDeletePlotTemplateFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSetAsDefaultTemplateFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateNewPlotFromTemplateFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSelectCaseOrEnsembleUi.cpp
)

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
