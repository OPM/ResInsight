set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensExportToFolderFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensExportToFolderUi.h
    ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensExportImpl.h
    ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensExportToSharingServerFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensCreateSessionFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensTerminateSessionFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensRestClient.h
    ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensServerSettings.h
    ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensCreateSessionUi.h
    ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensSession.h
    ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensSessionObserver.h
    ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensSessionManager.h
    ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensCreateDummyFileBackedSessionFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensAutoExportToSharingServerFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicExportToSharingServerScheduler.h
    ${CMAKE_CURRENT_LIST_DIR}/VdeArrayDataPacket.h
    ${CMAKE_CURRENT_LIST_DIR}/VdeCachingHashedIdFactory.h
    ${CMAKE_CURRENT_LIST_DIR}/VdeExportPart.h
    ${CMAKE_CURRENT_LIST_DIR}/VdeFileExporter.h
    ${CMAKE_CURRENT_LIST_DIR}/VdePacketDirectory.h
    ${CMAKE_CURRENT_LIST_DIR}/VdeVizDataExtractor.h)

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
    ${CMAKE_CURRENT_LIST_DIR}/VdeVizDataExtractor.cpp)

list(APPEND COMMAND_CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

set(COMMAND_QT_MOC_HEADERS
    ${COMMAND_QT_MOC_HEADERS} ${CMAKE_CURRENT_LIST_DIR}/RicHoloLensRestClient.h
    ${CMAKE_CURRENT_LIST_DIR}/RicExportToSharingServerScheduler.h)

source_group(
  "CommandFeature\\HoloLens"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake)
