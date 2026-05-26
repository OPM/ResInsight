set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimWellMeasurement.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellMeasurementCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellMeasurementFilePath.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellMeasurementCurve.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellMeasurementFilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellMeasurementInViewCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellMeasurementInView.cpp
)

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
