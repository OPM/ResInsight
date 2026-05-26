set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RiaPolyArcLineSampler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaWellPlanCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSCurveCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaArcCurveCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaJCurveCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaLineArcWellPathCalculator.cpp
)

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
