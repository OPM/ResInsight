set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RifTextDataTableFormatter.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RiaSocketServer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaProjectInfoCommands.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaCaseInfoCommands.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaGeometryCommands.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaNNCCommands.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaPropertyDataCommands.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaWellDataCommands.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSocketTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSocketDataTransfer.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
