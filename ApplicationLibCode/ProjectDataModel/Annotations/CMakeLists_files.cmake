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
    ${CMAKE_CURRENT_LIST_DIR}/RimPlotAxisAnnotation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimTimeAxisAnnotation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimTimeAxisAnnotationUpdater.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimEquilibriumAxisAnnotation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPlotRectAnnotation.cpp
)

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
