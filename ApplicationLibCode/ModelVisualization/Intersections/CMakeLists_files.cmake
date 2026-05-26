set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RivExtrudedCurveIntersectionGeometryGenerator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivExtrudedCurveIntersectionPartMgr.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivExtrudedCurveIntersectionSourceInfo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivIntersectionResultsColoringTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivBoxIntersectionGeometryGenerator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivBoxIntersectionPartMgr.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivBoxIntersectionSourceInfo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivSectionFlattener.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivEclipseIntersectionGrid.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivFemIntersectionGrid.cpp
)

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
