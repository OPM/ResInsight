set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimContourMapResolutionTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimContourMapProjection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimEclipseContourMapProjection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimEclipseContourMapView.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimEclipseContourMapViewCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimStatisticsContourMap.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimStatisticsContourMapProjection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimStatisticsContourMapView.cpp
)

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
