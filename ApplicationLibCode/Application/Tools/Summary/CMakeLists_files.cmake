set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryAddressAnalyzer.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryAddressModifier.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryCurveAddress.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryCurveDefinition.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryDefines.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryStringTools.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryTools.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryAddressAnalyzer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryAddressModifier.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryCurveAddress.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryCurveDefinition.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryDefines.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryStringTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryTools.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
