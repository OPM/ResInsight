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
    ${CMAKE_CURRENT_LIST_DIR}/RimGeoMechFaultReactivationResult.cpp
)

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
