set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEvent.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventPerf.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventValve.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventTubing.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventState.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventType.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventControl.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventKeyword.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventKeywordItem.h
    ${CMAKE_CURRENT_LIST_DIR}/RimKeywordEvent.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventTimeline.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEvent.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventPerf.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventValve.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventTubing.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventState.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventType.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventControl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventKeyword.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventKeywordItem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimKeywordEvent.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellEventTimeline.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
