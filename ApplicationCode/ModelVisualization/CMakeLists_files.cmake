
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RivCellEdgeEffectGenerator.h
${CEE_CURRENT_LIST_DIR}RivGridPartMgr.h
${CEE_CURRENT_LIST_DIR}RivReservoirPartMgr.h
${CEE_CURRENT_LIST_DIR}RivReservoirViewPartMgr.h
${CEE_CURRENT_LIST_DIR}RivPipeGeometryGenerator.h
${CEE_CURRENT_LIST_DIR}RivReservoirPipesPartMgr.h
${CEE_CURRENT_LIST_DIR}RivWellPathPartMgr.h
${CEE_CURRENT_LIST_DIR}RivWellPathCollectionPartMgr.h
${CEE_CURRENT_LIST_DIR}RivWellPipesPartMgr.h
${CEE_CURRENT_LIST_DIR}RivWellHeadPartMgr.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RivCellEdgeEffectGenerator.cpp
${CEE_CURRENT_LIST_DIR}RivGridPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivReservoirPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivReservoirViewPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivPipeGeometryGenerator.cpp
${CEE_CURRENT_LIST_DIR}RivReservoirPipesPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivWellPathPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivWellPathCollectionPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivWellPipesPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivWellHeadPartMgr.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "ModelVisualization" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} )
