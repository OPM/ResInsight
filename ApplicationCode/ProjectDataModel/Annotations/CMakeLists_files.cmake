
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RimAnnotationCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimPolylinesAnnotation.h
${CMAKE_CURRENT_LIST_DIR}/RimReachCircleAnnotation.h
${CMAKE_CURRENT_LIST_DIR}/RimTextAnnotation.h
${CMAKE_CURRENT_LIST_DIR}/RimAnnotationInViewCollection.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RimAnnotationCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimPolylinesAnnotation.cpp
${CMAKE_CURRENT_LIST_DIR}/RimReachCircleAnnotation.cpp
${CMAKE_CURRENT_LIST_DIR}/RimTextAnnotation.cpp
${CMAKE_CURRENT_LIST_DIR}/RimAnnotationInViewCollection.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

set (QT_MOC_HEADERS
${QT_MOC_HEADERS}
)


source_group( "ProjectDataModel\\Annotations" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
