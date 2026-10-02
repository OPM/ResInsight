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
)

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
