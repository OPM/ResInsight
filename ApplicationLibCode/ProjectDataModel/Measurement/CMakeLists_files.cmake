set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimMeasurement.h
    ${CMAKE_CURRENT_LIST_DIR}/RiuMeasurementEventFilter.h)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimMeasurement.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiuMeasurementEventFilter.cpp)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

set(QT_MOC_HEADERS ${CMAKE_CURRENT_LIST_DIR}/RiuMeasurementEventFilter.h
                   ${QT_MOC_HEADERS})

source_group(
  "ProjectDataModel\\Measurement"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake)
