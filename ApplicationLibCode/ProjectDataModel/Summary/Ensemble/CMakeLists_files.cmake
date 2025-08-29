set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryFileSetEnsemble.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryEnsembleParameter.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryEnsembleParameterCollection.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryFileSetEnsemble.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryEnsembleParameter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryEnsembleParameterCollection.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
