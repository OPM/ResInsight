set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RigEclipseResultCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RigSoilResultCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RigFaultDistanceResultCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RigMobilePoreVolumeResultCalculator.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RigEclipseResultCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigSoilResultCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigFaultDistanceResultCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigMobilePoreVolumeResultCalculator.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

source_group(
  "ReservoirDataModel\\ResultCalculators"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake
)
