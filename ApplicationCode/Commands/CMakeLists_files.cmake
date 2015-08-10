
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RicEclipseCaseClose.h
${CEE_CURRENT_LIST_DIR}RicEclipseCaseCopy.h
${CEE_CURRENT_LIST_DIR}RicEclipseCaseExecuteScript.h
${CEE_CURRENT_LIST_DIR}RicEclipseCaseNewGroup.h
${CEE_CURRENT_LIST_DIR}RicEclipseCaseNewView.h
${CEE_CURRENT_LIST_DIR}RicEclipseCasePaste.h
${CEE_CURRENT_LIST_DIR}RicEclipseCellResultSave.h
${CEE_CURRENT_LIST_DIR}RicEclipseFaultResultSave.h
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterNew.h
${CEE_CURRENT_LIST_DIR}RicEclipseViewCopy.h
${CEE_CURRENT_LIST_DIR}RicEclipseViewDelete.h
${CEE_CURRENT_LIST_DIR}RicEclipseViewNew.h
${CEE_CURRENT_LIST_DIR}RicEclipseViewPaste.h
#${CEE_CURRENT_LIST_DIR}RicGridModelsCreateCaseGroupFromFiles.h
#${CEE_CURRENT_LIST_DIR}RicGridModelsImport.h
#${CEE_CURRENT_LIST_DIR}RicGridModelsImportInput.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterDelete.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterInsert.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterNew.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterHelper.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewExec.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceI.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceJ.h
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceK.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RicEclipseCaseClose.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseCaseCopy.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseCaseExecuteScript.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseCaseNewGroup.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseCaseNewView.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseCasePaste.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseCellResultSave.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseFaultResultSave.cpp
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterNew.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseViewCopy.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseViewDelete.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseViewNew.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseViewPaste.cpp
#${CEE_CURRENT_LIST_DIR}RicGridModelsCreateCaseGroupFromFiles.cpp
#${CEE_CURRENT_LIST_DIR}RicGridModelsImport.cpp
#${CEE_CURRENT_LIST_DIR}RicGridModelsImportInput.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterDelete.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterInsert.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterNew.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterHelper.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewExec.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceI.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceJ.cpp
${CEE_CURRENT_LIST_DIR}RicRangeFilterNewSliceK.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
