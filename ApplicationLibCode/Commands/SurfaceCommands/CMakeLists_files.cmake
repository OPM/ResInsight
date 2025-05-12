set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicImportSurfacesFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicReloadSurfaceFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicCopySurfaceFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewGridCaseSurfaceFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicExportKLayerToPtlFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicExportSurfaceToTsurfFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewSurfaceCollectionFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewDepthSurfaceFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewRegularSurfaceFeature.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicImportSurfacesFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicReloadSurfaceFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCopySurfaceFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewGridCaseSurfaceFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExportKLayerToPtlFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExportSurfaceToTsurfFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewSurfaceCollectionFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewDepthSurfaceFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewRegularSurfaceFeature.cpp
)

list(APPEND COMMAND_CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
