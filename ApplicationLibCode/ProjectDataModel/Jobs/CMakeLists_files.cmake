set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimGenericJob.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimOpmFlowJob.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimOpmFlowJobSettings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimJobCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimDeckPositionDlg.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimKeywordFactory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimKeywordWconprod.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimKeywordWconinje.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimJobMonitor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimKeywordBcprop.cpp
)

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
