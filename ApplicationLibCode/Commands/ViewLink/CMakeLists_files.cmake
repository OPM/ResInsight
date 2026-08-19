set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicLinkVisibleViewsFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicLinkVisibleViewsFeatureUi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicLinkViewFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicUnLinkViewFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicShowLinkOptionsFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicDeleteAllLinkedViewsFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSetMasterViewFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicRemoveComparison3dViewFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCompareTo3dViewFeature.cpp
)

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
