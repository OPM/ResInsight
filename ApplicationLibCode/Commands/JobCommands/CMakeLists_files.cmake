set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicRunJobFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewOpmFlowJobFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicDuplicateJobFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicViewJobLogFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicStopJobFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicStopAllJobsFeature.cpp
)

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
