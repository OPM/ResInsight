set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimGeoMechModels.h
    ${CMAKE_CURRENT_LIST_DIR}/RimGeoMechCase.h
    ${CMAKE_CURRENT_LIST_DIR}/RimGeoMechView.h
    ${CMAKE_CURRENT_LIST_DIR}/RimGeoMechResultDefinition.h
    ${CMAKE_CURRENT_LIST_DIR}/RimGeoMechCellColors.h
    ${CMAKE_CURRENT_LIST_DIR}/RimGeoMechContourMapProjection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimGeoMechContourMapView.h
    ${CMAKE_CURRENT_LIST_DIR}/RimGeoMechContourMapViewCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimGeoMechPartCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimGeoMechPart.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimGeoMechModels.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimGeoMechCase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimGeoMechView.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimGeoMechResultDefinition.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimGeoMechCellColors.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimGeoMechContourMapProjection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimGeoMechContourMapView.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimGeoMechContourMapViewCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimGeoMechPartCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimGeoMechPart.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

source_group(
  "ProjectDataModel\\GeoMech"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake
)
