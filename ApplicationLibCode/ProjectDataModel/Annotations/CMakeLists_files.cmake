set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimAnnotationCollectionBase.h
    ${CMAKE_CURRENT_LIST_DIR}/RimAnnotationCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimPolylinesAnnotation.h
    ${CMAKE_CURRENT_LIST_DIR}/RimPolylinesFromFileAnnotation.h
    ${CMAKE_CURRENT_LIST_DIR}/RimUserDefinedPolylinesAnnotation.h
    ${CMAKE_CURRENT_LIST_DIR}/RimReachCircleAnnotation.h
    ${CMAKE_CURRENT_LIST_DIR}/RimTextAnnotation.h
    ${CMAKE_CURRENT_LIST_DIR}/RimAnnotationInViewCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimAnnotationLineAppearance.h
    ${CMAKE_CURRENT_LIST_DIR}/RimAnnotationTextAppearance.h
    ${CMAKE_CURRENT_LIST_DIR}/RimReachCircleAnnotationInView.h
    ${CMAKE_CURRENT_LIST_DIR}/RimTextAnnotationInView.h
    ${CMAKE_CURRENT_LIST_DIR}/RimAnnotationGroupCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimPolylineTarget.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimAnnotationCollectionBase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimAnnotationCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPolylinesAnnotation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPolylinesFromFileAnnotation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimUserDefinedPolylinesAnnotation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimReachCircleAnnotation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimTextAnnotation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimAnnotationInViewCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimAnnotationLineAppearance.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimAnnotationTextAppearance.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimReachCircleAnnotationInView.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimTextAnnotationInView.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimAnnotationGroupCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPolylineTarget.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
