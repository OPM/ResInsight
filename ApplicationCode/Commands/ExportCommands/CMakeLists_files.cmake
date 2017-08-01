
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RicExportCarfin.h
${CEE_CURRENT_LIST_DIR}RicExportCarfinUi.h
${CEE_CURRENT_LIST_DIR}RicExportToLasFileFeature.h
${CEE_CURRENT_LIST_DIR}RicExportToLasFileResampleUi.h
${CEE_CURRENT_LIST_DIR}RicSnapshotViewToClipboardFeature.h
${CEE_CURRENT_LIST_DIR}RicExportFaultsFeature.h
${CEE_CURRENT_LIST_DIR}RicExportMultipleSnapshotsFeature.h
${CEE_CURRENT_LIST_DIR}RicSaveEclipseInputPropertyFeature.h
${CEE_CURRENT_LIST_DIR}RicSaveEclipseResultAsInputPropertyExec.h
${CEE_CURRENT_LIST_DIR}RicSaveEclipseResultAsInputPropertyFeature.h
${CEE_CURRENT_LIST_DIR}RicCellRangeUi.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RicExportCarfin.cpp
${CEE_CURRENT_LIST_DIR}RicExportCarfinUi.cpp
${CEE_CURRENT_LIST_DIR}RicExportToLasFileFeature.cpp
${CEE_CURRENT_LIST_DIR}RicExportToLasFileResampleUi.cpp
${CEE_CURRENT_LIST_DIR}RicSnapshotViewToClipboardFeature.cpp
${CEE_CURRENT_LIST_DIR}RicExportFaultsFeature.cpp
${CEE_CURRENT_LIST_DIR}RicExportMultipleSnapshotsFeature.cpp
${CEE_CURRENT_LIST_DIR}RicSaveEclipseInputPropertyFeature.cpp
${CEE_CURRENT_LIST_DIR}RicSaveEclipseResultAsInputPropertyExec.cpp
${CEE_CURRENT_LIST_DIR}RicSaveEclipseResultAsInputPropertyFeature.cpp
${CEE_CURRENT_LIST_DIR}RicCellRangeUi.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\Export" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
