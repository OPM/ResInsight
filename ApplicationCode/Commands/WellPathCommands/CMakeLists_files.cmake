
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RicWellPathDeleteFeature.h
${CEE_CURRENT_LIST_DIR}RicWellPathExportCompletionDataFeature.h
${CEE_CURRENT_LIST_DIR}RicWellPathImportCompletionsFileFeature.h
${CEE_CURRENT_LIST_DIR}RicWellPathImportPerforationIntervalsFeature.h
${CEE_CURRENT_LIST_DIR}RicWellPathsImportFileFeature.h
${CEE_CURRENT_LIST_DIR}RicWellPathsImportSsihubFeature.h
${CEE_CURRENT_LIST_DIR}RicWellPathViewerEventHandler.h 
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RicWellPathDeleteFeature.cpp
${CEE_CURRENT_LIST_DIR}RicWellPathExportCompletionDataFeature.cpp
${CEE_CURRENT_LIST_DIR}RicWellPathImportCompletionsFileFeature.cpp
${CEE_CURRENT_LIST_DIR}RicWellPathImportPerforationIntervalsFeature.cpp
${CEE_CURRENT_LIST_DIR}RicWellPathsImportFileFeature.cpp
${CEE_CURRENT_LIST_DIR}RicWellPathsImportSsihubFeature.cpp
${CEE_CURRENT_LIST_DIR}RicWellPathViewerEventHandler.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\WellPath" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
