set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimPolygon.h
    ${CMAKE_CURRENT_LIST_DIR}/RimPolygonFile.h
    ${CMAKE_CURRENT_LIST_DIR}/RimPolygonCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimPolygonInView.h
    ${CMAKE_CURRENT_LIST_DIR}/RimPolygonInViewCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimPolygonAppearance.h
    ${CMAKE_CURRENT_LIST_DIR}/RimPolygonTools.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimPolygon.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPolygonFile.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPolygonCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPolygonInView.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPolygonInViewCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPolygonAppearance.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPolygonTools.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

