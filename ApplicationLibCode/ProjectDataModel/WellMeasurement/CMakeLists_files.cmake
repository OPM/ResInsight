
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RimWellMeasurement.h
${CMAKE_CURRENT_LIST_DIR}/RimWellMeasurementCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimWellMeasurementFilePath.h
${CMAKE_CURRENT_LIST_DIR}/RimWellMeasurementCurve.h
${CMAKE_CURRENT_LIST_DIR}/RimWellMeasurementFilter.h
${CMAKE_CURRENT_LIST_DIR}/RimWellMeasurementInViewCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimWellMeasurementInView.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RimWellMeasurement.cpp
${CMAKE_CURRENT_LIST_DIR}/RimWellMeasurementCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimWellMeasurementFilePath.cpp
${CMAKE_CURRENT_LIST_DIR}/RimWellMeasurementCurve.cpp
${CMAKE_CURRENT_LIST_DIR}/RimWellMeasurementFilter.cpp
${CMAKE_CURRENT_LIST_DIR}/RimWellMeasurementInViewCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimWellMeasurementInView.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "ProjectDataModel\\WellMeasurement" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
