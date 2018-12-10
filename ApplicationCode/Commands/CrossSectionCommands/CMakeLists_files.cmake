
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicAppendIntersectionFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewSimWellIntersectionFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewWellPathIntersectionFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewPolylineIntersectionFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewAzimuthDipIntersectionFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicCopyIntersectionsToAllViewsInCaseFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicAppendIntersectionFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewSimWellIntersectionFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewWellPathIntersectionFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewPolylineIntersectionFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewAzimuthDipIntersectionFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicCopyIntersectionsToAllViewsInCaseFeature.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\CrossSection" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
