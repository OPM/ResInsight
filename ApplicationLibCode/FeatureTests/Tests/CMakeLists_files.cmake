set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicDeleteWellPathFeature-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewViewFeature-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCloseCaseFeature-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicToggleItemsFeature-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewIntersectionFeature-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicEclipsePropertyFilterFeature-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewCellFilterFeature-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewWellPathLateralFeature-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewPerforationIntervalFeature-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewDefaultSummaryPlotFeature-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewWellLogPlotFeature-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCreatePolygonFeature-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewCellIndexFilterFeature-Test.cpp
)

list(APPEND SOURCE_FEATURETEST_FILES ${SOURCE_GROUP_SOURCE_FILES})
