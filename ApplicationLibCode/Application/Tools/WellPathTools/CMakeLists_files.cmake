set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RiaPolyArcLineSampler.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaWellPlanCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaSCurveCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaArcCurveCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaJCurveCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaLineArcWellPathCalculator.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RiaPolyArcLineSampler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaWellPlanCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSCurveCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaArcCurveCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaJCurveCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaLineArcWellPathCalculator.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
