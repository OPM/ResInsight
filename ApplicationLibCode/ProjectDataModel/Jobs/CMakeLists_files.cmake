set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimGenericJob.h
    ${CMAKE_CURRENT_LIST_DIR}/RimOpmFlowJob.h
    ${CMAKE_CURRENT_LIST_DIR}/RimJobCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimDeckPositionDlg.h
    ${CMAKE_CURRENT_LIST_DIR}/RimKeywordFactory.h
    ${CMAKE_CURRENT_LIST_DIR}/RimKeywordWconprod.h
    ${CMAKE_CURRENT_LIST_DIR}/RimKeywordWconinje.h
    ${CMAKE_CURRENT_LIST_DIR}/RimJobMonitor.h
    ${CMAKE_CURRENT_LIST_DIR}/RimKeywordBcprop.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimGenericJob.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimOpmFlowJob.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimJobCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimDeckPositionDlg.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimKeywordFactory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimKeywordWconprod.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimKeywordWconinje.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimJobMonitor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimKeywordBcprop.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
