
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicHoloLensExportToFolderFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicHoloLensExportToFolderUi.h
${CMAKE_CURRENT_LIST_DIR}/RicHoloLensExportImpl.h
${CMAKE_CURRENT_LIST_DIR}/RicHoloLensExportToSharingServerFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicHoloLensCreateSessionFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicHoloLensTerminateSessionFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicHoloLensServerSettings.h
${CMAKE_CURRENT_LIST_DIR}/RicHoloLensCreateSessionUi.h
${CMAKE_CURRENT_LIST_DIR}/RicHoloLensSession.h
${CMAKE_CURRENT_LIST_DIR}/RicHoloLensCreateDummyFileBackedSessionFeature.h

${CMAKE_CURRENT_LIST_DIR}/VdeArrayDataPacket.h
${CMAKE_CURRENT_LIST_DIR}/VdeExportPart.h
${CMAKE_CURRENT_LIST_DIR}/VdeFileExporter.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicHoloLensExportToFolderFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicHoloLensExportImpl.cpp
${CMAKE_CURRENT_LIST_DIR}/RicHoloLensExportToFolderUi.cpp 
${CMAKE_CURRENT_LIST_DIR}/RicHoloLensExportToSharingServerFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicHoloLensCreateSessionFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicHoloLensTerminateSessionFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicHoloLensServerSettings.cpp
${CMAKE_CURRENT_LIST_DIR}/RicHoloLensCreateSessionUi.cpp
${CMAKE_CURRENT_LIST_DIR}/RicHoloLensSession.cpp
${CMAKE_CURRENT_LIST_DIR}/RicHoloLensCreateDummyFileBackedSessionFeature.cpp

${CMAKE_CURRENT_LIST_DIR}/VdeArrayDataPacket.cpp
${CMAKE_CURRENT_LIST_DIR}/VdeExportPart.cpp
${CMAKE_CURRENT_LIST_DIR}/VdeFileExporter.cpp
)


list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\HoloLens" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
