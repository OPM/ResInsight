set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicShowPlotWindowFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicShowMainWindowFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicTileWindowsFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicOpenProjectFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicOpenLastUsedFileFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSaveProjectFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSaveProjectAsFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExitApplicationFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCloseProjectFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicHelpFeatures.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicEditPreferencesFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicShowPlotDataFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicLaunchRegressionTestsFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicRunCommandFileFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicShowMemoryCleanupDialogFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExportObjectAndFieldKeywordsFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSaveProjectNoGlobalPathsFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicShowClassNamesFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicOpenInTextEditorFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicShowMemoryReportFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSumoDataFeature.cpp
)

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
