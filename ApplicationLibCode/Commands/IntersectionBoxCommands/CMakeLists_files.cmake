set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicIntersectionFeatureImpl.h
    ${CMAKE_CURRENT_LIST_DIR}/RicBoxManipulatorEventHandler.h
    ${CMAKE_CURRENT_LIST_DIR}/RicAppendIntersectionBoxFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicIntersectionBoxXSliceFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicIntersectionBoxYSliceFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicIntersectionBoxZSliceFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicIntersectionBoxAtPosFeature.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicIntersectionFeatureImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicBoxManipulatorEventHandler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicAppendIntersectionBoxFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicIntersectionBoxXSliceFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicIntersectionBoxYSliceFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicIntersectionBoxZSliceFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicIntersectionBoxAtPosFeature.cpp
)

list(APPEND COMMAND_CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
