set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RiaCloudConnector.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaSumoConnector.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaSumoDefines.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaConnectorTools.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaOsduConnector.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaOAuthHttpServerReplyHandler.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RiaCloudConnector.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSumoConnector.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSumoDefines.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaConnectorTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaOsduConnector.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaOAuthHttpServerReplyHandler.cpp
)

list(
  APPEND
  QT_MOC_HEADERS
  ${CMAKE_CURRENT_LIST_DIR}/RiaCloudConnector.h
  ${CMAKE_CURRENT_LIST_DIR}/RiaSumoConnector.h
  ${CMAKE_CURRENT_LIST_DIR}/RiaOsduConnector.h
  ${CMAKE_CURRENT_LIST_DIR}/RiaOAuthHttpServerReplyHandler.h
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
