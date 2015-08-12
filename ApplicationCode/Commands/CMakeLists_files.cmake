
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RicEclipseCaseClose.h
${CEE_CURRENT_LIST_DIR}RicEclipseCaseCopy.h
${CEE_CURRENT_LIST_DIR}RicEclipseCaseExecuteScript.h
${CEE_CURRENT_LIST_DIR}RicEclipseCaseNewGroup.h
${CEE_CURRENT_LIST_DIR}RicEclipseCaseNewGroupExec.h
${CEE_CURRENT_LIST_DIR}RicEclipseCaseNewView.h
${CEE_CURRENT_LIST_DIR}RicEclipseCasePaste.h
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilter.h
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterInsert.h
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterInsertExec.h
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterNew.h
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterNewExec.h
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilter.h
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterInsert.h
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterInsertExec.h
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterNew.h
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterNewExec.h
${CEE_CURRENT_LIST_DIR}RicEclipseViewCopy.h
${CEE_CURRENT_LIST_DIR}RicEclipseViewDelete.h
${CEE_CURRENT_LIST_DIR}RicEclipseViewNew.h
${CEE_CURRENT_LIST_DIR}RicEclipseViewPaste.h
#${CEE_CURRENT_LIST_DIR}RicGridModelsCreateCaseGroupFromFiles.h
#${CEE_CURRENT_LIST_DIR}RicGridModelsImport.h
#${CEE_CURRENT_LIST_DIR}RicGridModelsImportInput.h
${CEE_CURRENT_LIST_DIR}RicPropertyFilterNewExec.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterInsert.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterNew.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterHelper.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewExec.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceI.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceJ.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceK.h
${CEE_CURRENT_LIST_DIR}RicSaveEclipseResultAsInputProperty.h
${CEE_CURRENT_LIST_DIR}RicSaveEclipseResultAsInputPropertyExec.h

${CEE_CURRENT_LIST_DIR}RicImportWellPathsSsihubFeature.h
${CEE_CURRENT_LIST_DIR}RicImportWellPathsFileFeature.h
${CEE_CURRENT_LIST_DIR}RicImportWellPathsDeleteAllFeature.h

# General delete of any object in a child array field
${CEE_CURRENT_LIST_DIR}RicDeleteItemExec.h
${CEE_CURRENT_LIST_DIR}RicDeleteItemExecData.h
${CEE_CURRENT_LIST_DIR}RicDeleteItemFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RicEclipseCaseClose.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseCaseCopy.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseCaseExecuteScript.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseCaseNewGroup.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseCaseNewGroupExec.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseCaseNewView.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseCasePaste.cpp
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilter.cpp
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterInsert.cpp
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterInsertExec.cpp
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterNew.cpp
${CEE_CURRENT_LIST_DIR}RicGeoMechPropertyFilterNewExec.cpp
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilter.cpp
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterInsert.cpp
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterInsertExec.cpp
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterNew.cpp
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterNewExec.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseViewCopy.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseViewDelete.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseViewNew.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseViewPaste.cpp
#${CEE_CURRENT_LIST_DIR}RicGridModelsCreateCaseGroupFromFiles.cpp
#${CEE_CURRENT_LIST_DIR}RicGridModelsImport.cpp
#${CEE_CURRENT_LIST_DIR}RicGridModelsImportInput.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterInsert.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterNew.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterHelper.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewExec.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceI.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceJ.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceK.cpp
${CEE_CURRENT_LIST_DIR}RicSaveEclipseResultAsInputProperty.cpp
${CEE_CURRENT_LIST_DIR}RicSaveEclipseResultAsInputPropertyExec.cpp

${CEE_CURRENT_LIST_DIR}RicImportWellPathsSsihubFeature.cpp
${CEE_CURRENT_LIST_DIR}RicImportWellPathsFileFeature.cpp
${CEE_CURRENT_LIST_DIR}RicImportWellPathsDeleteAllFeature.cpp

# General delete of any object in a child array field
${CEE_CURRENT_LIST_DIR}RicDeleteItemExec.cpp
${CEE_CURRENT_LIST_DIR}RicDeleteItemExecData.cpp
${CEE_CURRENT_LIST_DIR}RicDeleteItemFeature.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
