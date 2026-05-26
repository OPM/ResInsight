set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimPolygon.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPolygonContainer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPolygonFile.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPolygonCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPolygonInView.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPolygonInViewCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPolygonAppearance.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPolygonTools.cpp
)

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
