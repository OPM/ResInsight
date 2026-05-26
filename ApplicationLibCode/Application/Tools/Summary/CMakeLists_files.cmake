set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryAddressAnalyzer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryAddressModifier.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryCurveAddress.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryCurveDefinition.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryDefines.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryStringTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryPlotTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryPlotBuilder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryPlotTemplateTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryAddressCollectionTools.cpp
)

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
