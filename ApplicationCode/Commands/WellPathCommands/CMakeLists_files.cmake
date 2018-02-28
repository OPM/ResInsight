
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicWellPathDeleteFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicWellPathsImportFileFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicWellPathsImportSsihubFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicWellPathsUnitSystemSettingsImpl.h
${CMAKE_CURRENT_LIST_DIR}/RicWellPathsUnitSystemSettingsUi.h
${CMAKE_CURRENT_LIST_DIR}/RicWellPathViewerEventHandler.h 
${CMAKE_CURRENT_LIST_DIR}/RicIntersectionViewerEventHandler.h
${CMAKE_CURRENT_LIST_DIR}/RicWellPathFormationsImportFileFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicWellPathDeleteFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicWellPathsImportFileFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicWellPathsImportSsihubFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicWellPathsUnitSystemSettingsImpl.cpp
${CMAKE_CURRENT_LIST_DIR}/RicWellPathsUnitSystemSettingsUi.cpp
${CMAKE_CURRENT_LIST_DIR}/RicWellPathViewerEventHandler.cpp
${CMAKE_CURRENT_LIST_DIR}/RicIntersectionViewerEventHandler.cpp
${CMAKE_CURRENT_LIST_DIR}/RicWellPathFormationsImportFileFeature.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\WellPath" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
