
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RicLinkVisibleViewsFeature.h
${CEE_CURRENT_LIST_DIR}RicLinkVisibleViewsFeatureUi.h
${CEE_CURRENT_LIST_DIR}RicShowAllLinkedViewsFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RicLinkVisibleViewsFeature.cpp
${CEE_CURRENT_LIST_DIR}RicLinkVisibleViewsFeatureUi.cpp
${CEE_CURRENT_LIST_DIR}RicShowAllLinkedViewsFeature.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\ViewLink" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
