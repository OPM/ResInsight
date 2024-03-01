set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicCreatePolygonFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicImportPolygonFileFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicReloadPolygonFileFeature.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicCreatePolygonFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicImportPolygonFileFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicReloadPolygonFileFeature.cpp
)

list(APPEND COMMAND_CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

source_group(
  "CommandFeature\\Polygons"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake
)
