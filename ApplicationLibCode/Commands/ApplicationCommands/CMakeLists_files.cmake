set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicShowPlotWindowFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicShowMainWindowFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicTileWindowsFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicOpenProjectFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicOpenLastUsedFileFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicSaveProjectFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicSaveProjectAsFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicExitApplicationFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicCloseProjectFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicHelpFeatures.h
    ${CMAKE_CURRENT_LIST_DIR}/RicEditPreferencesFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicShowPlotDataFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicLaunchRegressionTestsFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicRunCommandFileFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicShowMemoryCleanupDialogFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicExportObjectAndFieldKeywordsFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicSaveProjectNoGlobalPathsFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicShowClassNamesFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicShowPlotDataCtxFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicOpenInTextEditorFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicShowMemoryReportFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicSumoDataFeature.h
)

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
    ${CMAKE_CURRENT_LIST_DIR}/RicShowPlotDataCtxFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicOpenInTextEditorFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicShowMemoryReportFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSumoDataFeature.cpp
)

list(APPEND COMMAND_CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

list(APPEND COMMAND_QT_MOC_HEADERS
     ${CMAKE_CURRENT_LIST_DIR}/RicSumoDataFeature.h
)

source_group(
  "CommandFeature\\Application"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake
)
