set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicCreatePolygonFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicImportPolygonFileFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicReloadPolygonFileFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicDuplicatePolygonFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicExportPolygonCsvFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicExportPolygonPolFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicSimplifyPolygonFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicBasicPolygonFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicDeleteAllPolygonsFeature.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicCreatePolygonFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicImportPolygonFileFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicReloadPolygonFileFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicDuplicatePolygonFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExportPolygonCsvFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExportPolygonPolFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSimplifyPolygonFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicBasicPolygonFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicDeleteAllPolygonsFeature.cpp
)

list(APPEND COMMAND_CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
