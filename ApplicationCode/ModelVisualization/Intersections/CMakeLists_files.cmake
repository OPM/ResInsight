
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RivCrossSectionGeometryGenerator.h
${CEE_CURRENT_LIST_DIR}RivCrossSectionPartMgr.h
${CEE_CURRENT_LIST_DIR}RivCrossSectionSourceInfo.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RivCrossSectionGeometryGenerator.cpp
${CEE_CURRENT_LIST_DIR}RivCrossSectionPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivCrossSectionSourceInfo.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "ModelVisualization\\Intersections" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
