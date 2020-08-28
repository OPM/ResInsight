
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicImportSurfacesFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicReloadSurfaceFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicCopySurfaceFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewGridCaseSurfaceFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicExportKLayerToPtlFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicExportSurfaceToTsurfFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewSurfaceCollectionFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicImportSurfacesFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicReloadSurfaceFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicCopySurfaceFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewGridCaseSurfaceFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicExportKLayerToPtlFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicExportSurfaceToTsurfFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewSurfaceCollectionFeature.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

#set (QT_MOC_HEADERS
#${QT_MOC_HEADERS}
#${CMAKE_CURRENT_LIST_DIR}/RicTextAnnotation3dEditor.h
#)


source_group( "CommandFeature\\SurfaceCommands" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
