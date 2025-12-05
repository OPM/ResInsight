set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RigEclipseResultCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RigSoilResultCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RigSwatResultCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RigFaultDistanceResultCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RigMobilePoreVolumeResultCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RigIndexIjkResultCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RigOilVolumeResultCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RigCellVolumeResultCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RigAllanUtil.h
    ${CMAKE_CURRENT_LIST_DIR}/RigCellsWithNncsCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RigPorvSoilSgasResultCalculator.cpp
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RigEclipseResultCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigSoilResultCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigSwatResultCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigFaultDistanceResultCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigMobilePoreVolumeResultCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigIndexIjkResultCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigOilVolumeResultCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigCellVolumeResultCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigAllanUtil.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigCellsWithNncsCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigPorvSoilSgasResultCalculator.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
