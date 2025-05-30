set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicCellRangeUi.h
    ${CMAKE_CURRENT_LIST_DIR}/RicExportCarfin.h
    ${CMAKE_CURRENT_LIST_DIR}/RicExportCarfinUi.h
    ${CMAKE_CURRENT_LIST_DIR}/RicExportFaultsFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicAdvancedSnapshotExportFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicExportToLasFileFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicExportToLasFileResampleUi.h
    ${CMAKE_CURRENT_LIST_DIR}/RicSaveEclipseInputPropertyFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicSaveEclipseInputVisibleCellsFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicSaveEclipseInputVisibleCellsUi.h
    ${CMAKE_CURRENT_LIST_DIR}/RicSaveEclipseResultAsInputPropertyExec.h
    ${CMAKE_CURRENT_LIST_DIR}/RicSaveEclipseResultAsInputPropertyFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicExportEclipseSectorModelFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicExportEclipseSectorModelUi.h
    ${CMAKE_CURRENT_LIST_DIR}/RicSnapshotAllPlotsToFileFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicSnapshotAllViewsToFileFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicSnapshotFilenameGenerator.h
    ${CMAKE_CURRENT_LIST_DIR}/RicSnapshotViewToClipboardFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicSnapshotViewToFileFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicSnapshotViewToPdfFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicExportSelectedWellPathsFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicExportVisibleWellPathsFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicExportWellPathsUi.h
    ${CMAKE_CURRENT_LIST_DIR}/RicExportLgrFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicExportLgrUi.h
    ${CMAKE_CURRENT_LIST_DIR}/RicEclipseCellResultToFileImpl.h
    ${CMAKE_CURRENT_LIST_DIR}/RicLgrSplitType.h
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateDepthAdjustedLasFilesFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateDepthAdjustedLasFilesUi.h
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateDepthAdjustedLasFilesImpl.h
    ${CMAKE_CURRENT_LIST_DIR}/RicExportInpFileFeature.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicCellRangeUi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExportCarfin.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExportCarfinUi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExportFaultsFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicAdvancedSnapshotExportFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExportToLasFileFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExportToLasFileResampleUi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSaveEclipseInputPropertyFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSaveEclipseInputVisibleCellsFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSaveEclipseInputVisibleCellsUi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExportEclipseSectorModelFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExportEclipseSectorModelUi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSaveEclipseResultAsInputPropertyExec.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSaveEclipseResultAsInputPropertyFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSnapshotAllPlotsToFileFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSnapshotAllViewsToFileFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSnapshotFilenameGenerator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSnapshotViewToClipboardFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSnapshotViewToFileFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSnapshotViewToPdfFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExportSelectedWellPathsFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExportVisibleWellPathsFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExportWellPathsUi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExportLgrFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExportLgrUi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicEclipseCellResultToFileImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateDepthAdjustedLasFilesFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateDepthAdjustedLasFilesUi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateDepthAdjustedLasFilesImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExportInpFileFeature.cpp
)

list(APPEND COMMAND_CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
