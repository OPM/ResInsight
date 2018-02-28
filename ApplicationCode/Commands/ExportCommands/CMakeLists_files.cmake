
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicCellRangeUi.h
${CMAKE_CURRENT_LIST_DIR}/RicExportCarfin.h
${CMAKE_CURRENT_LIST_DIR}/RicExportCarfinUi.h
${CMAKE_CURRENT_LIST_DIR}/RicExportFaultsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicExportMultipleSnapshotsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicExportToLasFileFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicExportToLasFileResampleUi.h
${CMAKE_CURRENT_LIST_DIR}/RicSaveEclipseInputPropertyFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicSaveEclipseInputVisibleCellsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicSaveEclipseInputVisibleCellsUi.h
${CMAKE_CURRENT_LIST_DIR}/RicSaveEclipseResultAsInputPropertyExec.h
${CMAKE_CURRENT_LIST_DIR}/RicSaveEclipseResultAsInputPropertyFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicSnapshotAllPlotsToFileFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicSnapshotAllViewsToFileFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicSnapshotFilenameGenerator.h
${CMAKE_CURRENT_LIST_DIR}/RicSnapshotViewToClipboardFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicSnapshotViewToFileFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicCellRangeUi.cpp
${CMAKE_CURRENT_LIST_DIR}/RicExportCarfin.cpp
${CMAKE_CURRENT_LIST_DIR}/RicExportCarfinUi.cpp
${CMAKE_CURRENT_LIST_DIR}/RicExportFaultsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicExportMultipleSnapshotsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicExportToLasFileFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicExportToLasFileResampleUi.cpp
${CMAKE_CURRENT_LIST_DIR}/RicSaveEclipseInputPropertyFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicSaveEclipseInputVisibleCellsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicSaveEclipseInputVisibleCellsUi.cpp
${CMAKE_CURRENT_LIST_DIR}/RicSaveEclipseResultAsInputPropertyExec.cpp
${CMAKE_CURRENT_LIST_DIR}/RicSaveEclipseResultAsInputPropertyFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicSnapshotAllPlotsToFileFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicSnapshotAllViewsToFileFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicSnapshotFilenameGenerator.cpp
${CMAKE_CURRENT_LIST_DIR}/RicSnapshotViewToClipboardFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicSnapshotViewToFileFeature.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\Export" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
