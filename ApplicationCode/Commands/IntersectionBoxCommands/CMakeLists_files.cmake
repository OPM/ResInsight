
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicBoxManipulatorEventHandler.h
${CMAKE_CURRENT_LIST_DIR}/RicAppendIntersectionBoxFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicIntersectionBoxXSliceFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicIntersectionBoxYSliceFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicIntersectionBoxZSliceFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicIntersectionBoxAtPosFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicBoxManipulatorEventHandler.cpp
${CMAKE_CURRENT_LIST_DIR}/RicAppendIntersectionBoxFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicIntersectionBoxXSliceFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicIntersectionBoxYSliceFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicIntersectionBoxZSliceFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicIntersectionBoxAtPosFeature.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

set (QT_MOC_HEADERS
${QT_MOC_HEADERS}
${CMAKE_CURRENT_LIST_DIR}/RicBoxManipulatorEventHandler.h
)


source_group( "CommandFeature\\IntersectionBox" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
