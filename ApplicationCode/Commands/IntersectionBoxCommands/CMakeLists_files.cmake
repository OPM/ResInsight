
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RicBoxManipulatorEventHandler.h
${CEE_CURRENT_LIST_DIR}RicEditIntersectionBoxFeature.h
${CEE_CURRENT_LIST_DIR}RicAppendIntersectionBoxFeature.h
${CEE_CURRENT_LIST_DIR}RicIntersectionBoxXSliceFeature.h
${CEE_CURRENT_LIST_DIR}RicIntersectionBoxYSliceFeature.h
${CEE_CURRENT_LIST_DIR}RicIntersectionBoxZSliceFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RicBoxManipulatorEventHandler.cpp
${CEE_CURRENT_LIST_DIR}RicEditIntersectionBoxFeature.cpp
${CEE_CURRENT_LIST_DIR}RicAppendIntersectionBoxFeature.cpp
${CEE_CURRENT_LIST_DIR}RicIntersectionBoxXSliceFeature.cpp
${CEE_CURRENT_LIST_DIR}RicIntersectionBoxYSliceFeature.cpp
${CEE_CURRENT_LIST_DIR}RicIntersectionBoxZSliceFeature.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

set (QT_MOC_HEADERS
${QT_MOC_HEADERS}
${CEE_CURRENT_LIST_DIR}RicBoxManipulatorEventHandler.h
${CEE_CURRENT_LIST_DIR}RicEditIntersectionBoxFeature.h
)


source_group( "CommandFeature\\IntersectionBox" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
