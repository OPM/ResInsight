set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimBoxIntersection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimIntersection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimExtrudedCurveIntersection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimIntersectionCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimIntersectionResultDefinition.h
    ${CMAKE_CURRENT_LIST_DIR}/RimIntersectionResultsDefinitionCollection.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimIntersection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimExtrudedCurveIntersection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimIntersectionCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimIntersectionResultDefinition.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimIntersectionResultsDefinitionCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimBoxIntersection.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

source_group(
  "ProjectDataModel\\Intersection"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake
)
