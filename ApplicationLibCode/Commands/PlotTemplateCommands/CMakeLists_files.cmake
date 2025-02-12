set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicSelectPlotTemplateUi.h
    ${CMAKE_CURRENT_LIST_DIR}/RicReloadPlotTemplatesFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicCreatePlotFromTemplateByShortcutFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicSaveMultiPlotTemplateFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicSaveMultiPlotTemplateFeatureSettings.h
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateMultiPlotFromSelectionFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicRenamePlotTemplateFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicDeletePlotTemplateFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicSetAsDefaultTemplateFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateNewPlotFromTemplateFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicSelectCaseOrEnsembleUi.h
)

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

list(APPEND COMMAND_CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
