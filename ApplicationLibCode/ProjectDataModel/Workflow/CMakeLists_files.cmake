set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflow.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowDescribeTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowJob.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowTaskInput.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowFieldBinding.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowArrayBinding.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowStringBinding.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowFloatBinding.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowIntBinding.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowBoolBinding.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowCaseBinding.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowWellPathBinding.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowViewBinding.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowDateBinding.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowFilePathBinding.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowVec3Binding.cpp
)

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

source_group(
  "ProjectDataModel\\Workflow"
  FILES ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake
)
