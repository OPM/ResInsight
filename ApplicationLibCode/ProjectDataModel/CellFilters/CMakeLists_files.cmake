set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimCellFilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimCellFilterCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimCellFilterTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimCellRangeFilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimCombinedFilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimDataFilterCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimDataFilterInView.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimDataFilterInViewCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPropertyFilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPropertyFilterCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimEclipsePropertyFilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimEclipsePropertyFilterCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimGeoMechPropertyFilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimGeoMechPropertyFilterCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPolygonFilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimUserDefinedFilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimCellFilterIntervalTool.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimCellIndexFilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimUserDefinedIndexFilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFilterInViewCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFilterDisplayUtil.cpp
)

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
