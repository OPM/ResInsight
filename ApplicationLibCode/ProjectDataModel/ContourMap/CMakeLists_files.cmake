set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimContourMapResolutionTools.h
    ${CMAKE_CURRENT_LIST_DIR}/RimContourMapProjection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimEclipseContourMapProjection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimEclipseContourMapView.h
    ${CMAKE_CURRENT_LIST_DIR}/RimEclipseContourMapViewCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimStatisticsContourMap.h
    ${CMAKE_CURRENT_LIST_DIR}/RimStatisticsContourMapProjection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimStatisticsContourMapView.h
)

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

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
