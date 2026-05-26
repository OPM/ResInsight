set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicImportSurfacesFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicReloadSurfaceFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCopySurfaceFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewGridCaseSurfaceFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExportKLayerToPtlFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExportSurfaceToTsurfFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExportSurfaceToGriFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExportSurfaceToGriUi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewDepthSurfaceFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewRegularSurfaceFeature.cpp
)

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
