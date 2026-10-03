set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicNewWorkflowJobFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicRunWorkflowJobFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicCancelWorkflowJobFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicWorkflowLocationUi.h
    ${CMAKE_CURRENT_LIST_DIR}/RicWorkflowFeatureTools.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewWorkflowFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicDuplicateWorkflowAsEditableFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicSaveWorkflowFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicSaveWorkflowAsFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicRescanWorkflowsFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicAddWorkflowTaskFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicDeleteWorkflowTaskFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicDeleteWorkflowConnectionFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicRenameWorkflowTaskFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicSetWorkflowResultTaskFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicToggleWorkflowOptionalInputFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicMapWorkflowTaskFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicRemoveWorkflowTaskMapFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicRenameWorkflowCollectKeyFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicMoveWorkflowCollectMemberFeature.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicNewWorkflowJobFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicRunWorkflowJobFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCancelWorkflowJobFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicWorkflowLocationUi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicWorkflowFeatureTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewWorkflowFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicDuplicateWorkflowAsEditableFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSaveWorkflowFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSaveWorkflowAsFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicRescanWorkflowsFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicAddWorkflowTaskFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicDeleteWorkflowTaskFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicDeleteWorkflowConnectionFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicRenameWorkflowTaskFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSetWorkflowResultTaskFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicToggleWorkflowOptionalInputFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicMapWorkflowTaskFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicRemoveWorkflowTaskMapFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicRenameWorkflowCollectKeyFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicMoveWorkflowCollectMemberFeature.cpp
)

list(APPEND COMMAND_CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
