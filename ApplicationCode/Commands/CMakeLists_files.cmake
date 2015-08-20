
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RicCloseCaseFeature.h
${CEE_CURRENT_LIST_DIR}RicEditScriptFeature.h
${CEE_CURRENT_LIST_DIR}RicScriptFeatureImpl.h
${CEE_CURRENT_LIST_DIR}RicNewScriptFeature.h
${CEE_CURRENT_LIST_DIR}RicAddScriptPathFeature.h
${CEE_CURRENT_LIST_DIR}RicDeleteScriptPathFeature.h
${CEE_CURRENT_LIST_DIR}RicExecuteScriptFeature.h
${CEE_CURRENT_LIST_DIR}RicExecuteScriptForCasesFeature.h
${CEE_CURRENT_LIST_DIR}RicEclipseCaseNewGroupFeature.h
${CEE_CURRENT_LIST_DIR}RicEclipseCaseNewGroupExec.h
${CEE_CURRENT_LIST_DIR}RicEclipseCasePasteFeature.h
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterFeatureImpl.h
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterInsertFeature.h
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterInsertExec.h
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterNewFeature.h
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterNewExec.h
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterFeatureImpl.h
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterInsertFeature.h
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterInsertExec.h
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterNewFeature.h
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterNewExec.h
${CEE_CURRENT_LIST_DIR}RicNewViewFeature.h
${CEE_CURRENT_LIST_DIR}RicEclipseViewPasteFeature.h
#${CEE_CURRENT_LIST_DIR}RicGridModelsCreateCaseGroupFromFiles.h
#${CEE_CURRENT_LIST_DIR}RicGridModelsImport.h
#${CEE_CURRENT_LIST_DIR}RicGridModelsImportInput.h
${CEE_CURRENT_LIST_DIR}RicPropertyFilterNewExec.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterInsertFeature.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewFeature.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterFeatureImpl.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewExec.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceIFeature.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceJFeature.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceKFeature.h
${CEE_CURRENT_LIST_DIR}RicSaveEclipseResultAsInputPropertyFeature.h
${CEE_CURRENT_LIST_DIR}RicSaveEclipseResultAsInputPropertyExec.h
${CEE_CURRENT_LIST_DIR}RicImportEclipseCaseFeature.h

${CEE_CURRENT_LIST_DIR}RicWellPathsImportSsihubFeature.h
${CEE_CURRENT_LIST_DIR}RicWellPathsImportFileFeature.h
${CEE_CURRENT_LIST_DIR}RicWellPathsDeleteAllFeature.h

# General delete of any object in a child array field
${CEE_CURRENT_LIST_DIR}RicDeleteItemExec.h
${CEE_CURRENT_LIST_DIR}RicDeleteItemExecData.h
${CEE_CURRENT_LIST_DIR}RicDeleteItemFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RicCloseCaseFeature.cpp
${CEE_CURRENT_LIST_DIR}RicScriptFeatureImpl.cpp
${CEE_CURRENT_LIST_DIR}RicEditScriptFeature.cpp
${CEE_CURRENT_LIST_DIR}RicNewScriptFeature.cpp
${CEE_CURRENT_LIST_DIR}RicAddScriptPathFeature.cpp
${CEE_CURRENT_LIST_DIR}RicDeleteScriptPathFeature.cpp
${CEE_CURRENT_LIST_DIR}RicExecuteScriptFeature.cpp
${CEE_CURRENT_LIST_DIR}RicExecuteScriptForCasesFeature.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseCaseNewGroupFeature.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseCaseNewGroupExec.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseCasePasteFeature.cpp
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterFeatureImpl.cpp
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterInsertFeature.cpp
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterInsertExec.cpp
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterNewFeature.cpp
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterNewExec.cpp
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterFeatureImpl.cpp
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterInsertFeature.cpp
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterInsertExec.cpp
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterNewFeature.cpp
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterNewExec.cpp
${CEE_CURRENT_LIST_DIR}RicNewViewFeature.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseViewPasteFeature.cpp
#${CEE_CURRENT_LIST_DIR}RicGridModelsCreateCaseGroupFromFiles.cpp
#${CEE_CURRENT_LIST_DIR}RicGridModelsImport.cpp
#${CEE_CURRENT_LIST_DIR}RicGridModelsImportInput.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterInsertFeature.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewFeature.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterFeatureImpl.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewExec.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceIFeature.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceJFeature.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceKFeature.cpp
${CEE_CURRENT_LIST_DIR}RicSaveEclipseResultAsInputPropertyFeature.cpp
${CEE_CURRENT_LIST_DIR}RicSaveEclipseResultAsInputPropertyExec.cpp
${CEE_CURRENT_LIST_DIR}RicImportEclipseCaseFeature.cpp

${CEE_CURRENT_LIST_DIR}RicWellPathsImportSsihubFeature.cpp
${CEE_CURRENT_LIST_DIR}RicWellPathsImportFileFeature.cpp
${CEE_CURRENT_LIST_DIR}RicWellPathsDeleteAllFeature.cpp

# General delete of any object in a child array field
${CEE_CURRENT_LIST_DIR}RicDeleteItemExec.cpp
${CEE_CURRENT_LIST_DIR}RicDeleteItemExecData.cpp
${CEE_CURRENT_LIST_DIR}RicDeleteItemFeature.cpp
)

set (QT_MOC_HEADERS
${QT_MOC_HEADERS}
${CEE_CURRENT_LIST_DIR}RicExecuteScriptForCasesFeature.h
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
