set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEvent.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventPerf.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventRawText.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventValve.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventTubing.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventState.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventType.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventWellSpec.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventControl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventKeyword.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventKeywordItem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimKeywordEvent.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventTimeline.cpp
)

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
