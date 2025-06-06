set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicToggleItemsFeatureImpl.h
    ${CMAKE_CURRENT_LIST_DIR}/RicToggleItemsOnFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicToggleItemsOffFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicToggleItemsFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicToggleItemsOnOthersOffFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicCollapseSiblingsFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicMoveItemsToTopFeature.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicToggleItemsFeatureImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicToggleItemsOnFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicToggleItemsOffFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicToggleItemsFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicToggleItemsOnOthersOffFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCollapseSiblingsFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicMoveItemsToTopFeature.cpp
)

list(APPEND COMMAND_CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
