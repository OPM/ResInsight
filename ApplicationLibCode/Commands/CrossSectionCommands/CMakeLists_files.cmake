set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicAppendIntersectionFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicAppendSeparateIntersectionResultFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewSimWellIntersectionFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewWellPathIntersectionFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewPolylineIntersectionFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewAzimuthDipIntersectionFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicCopyIntersectionsToAllViewsInCaseFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewPolygonIntersectionFeature.h
)

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

list(APPEND COMMAND_CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
