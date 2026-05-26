set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicAppendIntersectionFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicAppendSeparateIntersectionResultFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewSimWellIntersectionFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewWellPathIntersectionFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewPolylineIntersectionFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewAzimuthDipIntersectionFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCopyIntersectionsToAllViewsInCaseFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewPolygonIntersectionFeature.cpp
)

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
