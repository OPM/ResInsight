
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicToggleItemsFeatureImpl.h
${CMAKE_CURRENT_LIST_DIR}/RicToggleItemsOnFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicToggleItemsOffFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicToggleItemsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicToggleItemsOnOthersOffFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicToggleItemsFeatureImpl.cpp
${CMAKE_CURRENT_LIST_DIR}/RicToggleItemsOnFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicToggleItemsOffFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicToggleItemsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicToggleItemsOnOthersOffFeature.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\ToggleItems" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
