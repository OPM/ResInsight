set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflow.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowJob.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowTaskInput.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowFieldBinding.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowStringBinding.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowFloatBinding.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowIntBinding.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowBoolBinding.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowCaseBinding.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowWellPathBinding.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowViewBinding.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowDateBinding.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowFilePathBinding.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflow.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowJob.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowTaskInput.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowFieldBinding.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowStringBinding.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowFloatBinding.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowIntBinding.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowBoolBinding.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowCaseBinding.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowWellPathBinding.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowViewBinding.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowDateBinding.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWorkflowFilePathBinding.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

source_group(
  "ProjectDataModel\\Workflow"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake
)
