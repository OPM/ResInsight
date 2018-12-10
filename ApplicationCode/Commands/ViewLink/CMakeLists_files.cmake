
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicLinkVisibleViewsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicLinkVisibleViewsFeatureUi.h
${CMAKE_CURRENT_LIST_DIR}/RicShowAllLinkedViewsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicLinkViewFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicUnLinkViewFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicShowLinkOptionsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicDeleteAllLinkedViewsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicSetMasterViewFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicLinkVisibleViewsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicLinkVisibleViewsFeatureUi.cpp
${CMAKE_CURRENT_LIST_DIR}/RicShowAllLinkedViewsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicLinkViewFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicUnLinkViewFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicShowLinkOptionsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicDeleteAllLinkedViewsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicSetMasterViewFeature.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\ViewLink" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
