set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimIntersection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimExtrudedCurveIntersection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimIntersectionCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimIntersectionResultDefinition.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimIntersectionResultsDefinitionCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimBoxIntersection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimIntersectionEnums.cpp
)

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
