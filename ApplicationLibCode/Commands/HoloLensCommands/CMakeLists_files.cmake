set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensExportToFolderFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensExportImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensExportToFolderUi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensExportToSharingServerFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensCreateSessionFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensTerminateSessionFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensRestClient.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensServerSettings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensCreateSessionUi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensSession.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensSessionManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensCreateDummyFileBackedSessionFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensAutoExportToSharingServerFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExportToSharingServerScheduler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/VdeArrayDataPacket.cpp
    ${CMAKE_CURRENT_LIST_DIR}/VdeCachingHashedIdFactory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/VdeExportPart.cpp
    ${CMAKE_CURRENT_LIST_DIR}/VdeFileExporter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/VdePacketDirectory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/VdeVizDataExtractor.cpp
)

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
