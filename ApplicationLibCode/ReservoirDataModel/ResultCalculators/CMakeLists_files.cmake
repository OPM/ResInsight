set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RigEclipseResultCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigSoilResultCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigSwatResultCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigSgasResultCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigFaultDistanceCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigFaultDistanceResultCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigSelectedFaultDistanceResultCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigMobilePoreVolumeResultCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigIndexIjkResultCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigOilVolumeResultCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigCellVolumeResultCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigAllanUtil.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigCellsWithNncsCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigPorvSoilSgasResultCalculator.cpp
)

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
