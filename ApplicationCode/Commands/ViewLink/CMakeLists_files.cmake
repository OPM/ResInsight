
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RicLinkVisibleViewsFeature.h
${CEE_CURRENT_LIST_DIR}RicLinkVisibleViewsFeatureUi.h
${CEE_CURRENT_LIST_DIR}RicShowAllLinkedViewsFeature.h
${CEE_CURRENT_LIST_DIR}RicLinkViewFeature.h
${CEE_CURRENT_LIST_DIR}RicUnLinkViewFeature.h
${CEE_CURRENT_LIST_DIR}RicShowLinkOptionsFeature.h
${CEE_CURRENT_LIST_DIR}RicDeleteAllLinkedViewsFeature.h
${CEE_CURRENT_LIST_DIR}RicSetMasterViewFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RicLinkVisibleViewsFeature.cpp
${CEE_CURRENT_LIST_DIR}RicLinkVisibleViewsFeatureUi.cpp
${CEE_CURRENT_LIST_DIR}RicShowAllLinkedViewsFeature.cpp
${CEE_CURRENT_LIST_DIR}RicLinkViewFeature.cpp
${CEE_CURRENT_LIST_DIR}RicUnLinkViewFeature.cpp
${CEE_CURRENT_LIST_DIR}RicShowLinkOptionsFeature.cpp
${CEE_CURRENT_LIST_DIR}RicDeleteAllLinkedViewsFeature.cpp
${CEE_CURRENT_LIST_DIR}RicSetMasterViewFeature.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\ViewLink" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
