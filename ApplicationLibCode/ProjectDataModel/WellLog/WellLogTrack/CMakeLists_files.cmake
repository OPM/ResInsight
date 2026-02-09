set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogTrack.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogPropertyAxisSettings.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogFormationSettings.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogRegionAnnotationSettings.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogWellPathAttributeSettings.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogTrack.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogPropertyAxisSettings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogFormationSettings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogRegionAnnotationSettings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogWellPathAttributeSettings.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
