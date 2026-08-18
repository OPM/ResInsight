set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RiaCloudApiService.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaCloudConnector.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSumoBlobCache.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSumoConnector.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSumoDefines.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSumoExplore.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSumoGrid.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSumoSummary.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaConnectorTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaOsduConnector.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaOAuthHttpServerReplyHandler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifReaderSumoGridProperty.cpp
)

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
