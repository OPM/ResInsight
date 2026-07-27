set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicIntersectionFeatureImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicBoxManipulatorEventHandler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicAppendIntersectionBoxFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicAppendIjkIntersectionFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewIjkIntersection3dviewFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicIntersectionBoxXSliceFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicIntersectionBoxYSliceFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicIntersectionBoxZSliceFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicIntersectionBoxAtPosFeature.cpp
)

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
