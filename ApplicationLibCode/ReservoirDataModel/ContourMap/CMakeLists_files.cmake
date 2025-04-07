set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RigContourMapCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RigContourMapGrid.h
    ${CMAKE_CURRENT_LIST_DIR}/RigContourPolygonsTools.h
    ${CMAKE_CURRENT_LIST_DIR}/RigContourMapCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RigContourMapProjection.h
    ${CMAKE_CURRENT_LIST_DIR}/RigContourMapTrianglesGenerator.h
    ${CMAKE_CURRENT_LIST_DIR}/RigEclipseContourMapProjection.h
    ${CMAKE_CURRENT_LIST_DIR}/RigGeoMechContourMapProjection.h
    ${CMAKE_CURRENT_LIST_DIR}/RigStatisticsContourMapProjection.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RigContourMapCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigContourMapGrid.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigContourPolygonsTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigContourMapCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigContourMapProjection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigContourMapTrianglesGenerator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigEclipseContourMapProjection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigGeoMechContourMapProjection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigStatisticsContourMapProjection.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
