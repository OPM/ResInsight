set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimFieldQuickAccess.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFieldQuickAccessGroup.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFieldQuickAccessInterface.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFieldReference.h
    ${CMAKE_CURRENT_LIST_DIR}/RimQuickAccessCollection.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimFieldQuickAccess.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFieldQuickAccessGroup.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFieldReference.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimQuickAccessCollection.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
