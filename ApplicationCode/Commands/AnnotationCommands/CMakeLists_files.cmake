
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicImportPolylinesAnnotationFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicCreateTextAnnotationFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicCreateTextAnnotationIn3dViewFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicCreateReachCircleAnnotationFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicCreateUserDefinedPolylinesAnnotationFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicTextAnnotation3dEditor.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicImportPolylinesAnnotationFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicCreateTextAnnotationFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicCreateTextAnnotationIn3dViewFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicCreateReachCircleAnnotationFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicCreateUserDefinedPolylinesAnnotationFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicTextAnnotation3dEditor.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

set (QT_MOC_HEADERS
${QT_MOC_HEADERS}
${CMAKE_CURRENT_LIST_DIR}/RicTextAnnotation3dEditor.h
)


source_group( "CommandFeature\\AnnotationCommands" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
