
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RivExtrudedCurveIntersectionGeometryGenerator.h
${CMAKE_CURRENT_LIST_DIR}/RivExtrudedCurveIntersectionPartMgr.h
${CMAKE_CURRENT_LIST_DIR}/RivIntersectionSourceInfo.h
${CMAKE_CURRENT_LIST_DIR}/RivIntersectionResultsColoringTools.h
${CMAKE_CURRENT_LIST_DIR}/RivHexGridIntersectionTools.h
${CMAKE_CURRENT_LIST_DIR}/RivIntersectionBoxGeometryGenerator.h
${CMAKE_CURRENT_LIST_DIR}/RivIntersectionBoxPartMgr.h
${CMAKE_CURRENT_LIST_DIR}/RivIntersectionBoxSourceInfo.h
${CMAKE_CURRENT_LIST_DIR}/RivSectionFlattner.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RivExtrudedCurveIntersectionGeometryGenerator.cpp
${CMAKE_CURRENT_LIST_DIR}/RivExtrudedCurveIntersectionPartMgr.cpp
${CMAKE_CURRENT_LIST_DIR}/RivIntersectionSourceInfo.cpp
${CMAKE_CURRENT_LIST_DIR}/RivIntersectionResultsColoringTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RivHexGridIntersectionTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RivIntersectionBoxGeometryGenerator.cpp
${CMAKE_CURRENT_LIST_DIR}/RivIntersectionBoxPartMgr.cpp
${CMAKE_CURRENT_LIST_DIR}/RivIntersectionBoxSourceInfo.cpp
${CMAKE_CURRENT_LIST_DIR}/RivSectionFlattner.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "ModelVisualization\\Intersections" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
