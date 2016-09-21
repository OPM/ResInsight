
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RivIntersectionGeometryGenerator.h
${CEE_CURRENT_LIST_DIR}RivIntersectionPartMgr.h
${CEE_CURRENT_LIST_DIR}RivCrossSectionSourceInfo.h
${CEE_CURRENT_LIST_DIR}RivHexGridIntersectionTools.h
${CEE_CURRENT_LIST_DIR}RivIntersectionBoxGeometryGenerator.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RivIntersectionGeometryGenerator.cpp
${CEE_CURRENT_LIST_DIR}RivIntersectionPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivCrossSectionSourceInfo.cpp
${CEE_CURRENT_LIST_DIR}RivHexGridIntersectionTools.cpp
${CEE_CURRENT_LIST_DIR}RivIntersectionBoxGeometryGenerator.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "ModelVisualization\\Intersections" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
