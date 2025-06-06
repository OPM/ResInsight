set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RivExtrudedCurveIntersectionGeometryGenerator.h
    ${CMAKE_CURRENT_LIST_DIR}/RivExtrudedCurveIntersectionPartMgr.h
    ${CMAKE_CURRENT_LIST_DIR}/RivExtrudedCurveIntersectionSourceInfo.h
    ${CMAKE_CURRENT_LIST_DIR}/RivIntersectionResultsColoringTools.h
    ${CMAKE_CURRENT_LIST_DIR}/RivHexGridIntersectionTools.h
    ${CMAKE_CURRENT_LIST_DIR}/RivBoxIntersectionGeometryGenerator.h
    ${CMAKE_CURRENT_LIST_DIR}/RivBoxIntersectionPartMgr.h
    ${CMAKE_CURRENT_LIST_DIR}/RivBoxIntersectionSourceInfo.h
    ${CMAKE_CURRENT_LIST_DIR}/RivSectionFlattener.h
    ${CMAKE_CURRENT_LIST_DIR}/RivEclipseIntersectionGrid.h
    ${CMAKE_CURRENT_LIST_DIR}/RivFemIntersectionGrid.h
    ${CMAKE_CURRENT_LIST_DIR}/RivIntersectionGeometryGeneratorInterface.h
)

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

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
