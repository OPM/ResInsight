set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateTextAnnotationFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateTextAnnotationIn3dViewFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateReachCircleAnnotationFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicTextAnnotation3dEditor.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateTextAnnotationFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateTextAnnotationIn3dViewFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateReachCircleAnnotationFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicTextAnnotation3dEditor.cpp
)

list(APPEND COMMAND_CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
