
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RivCellEdgeEffectGenerator.h
${CEE_CURRENT_LIST_DIR}RivColorTableArray.h
${CEE_CURRENT_LIST_DIR}RivFaultPartMgr.h
${CEE_CURRENT_LIST_DIR}RivFaultGeometryGenerator.h
${CEE_CURRENT_LIST_DIR}RivNNCGeometryGenerator.h
${CEE_CURRENT_LIST_DIR}RivGridPartMgr.h
${CEE_CURRENT_LIST_DIR}RivTernarySaturationOverlayItem.h
${CEE_CURRENT_LIST_DIR}RivReservoirPartMgr.h
${CEE_CURRENT_LIST_DIR}RivReservoirViewPartMgr.h
${CEE_CURRENT_LIST_DIR}RivPipeGeometryGenerator.h
${CEE_CURRENT_LIST_DIR}RivReservoirFaultsPartMgr.h
${CEE_CURRENT_LIST_DIR}RivReservoirPipesPartMgr.h
${CEE_CURRENT_LIST_DIR}RivSourceInfo.h
${CEE_CURRENT_LIST_DIR}RivWellPathSourceInfo.h
${CEE_CURRENT_LIST_DIR}RivWellPathPartMgr.h
${CEE_CURRENT_LIST_DIR}RivWellPathCollectionPartMgr.h
${CEE_CURRENT_LIST_DIR}RivWellPipesPartMgr.h
${CEE_CURRENT_LIST_DIR}RivWellHeadPartMgr.h
${CEE_CURRENT_LIST_DIR}RivResultToTextureMapper.h
${CEE_CURRENT_LIST_DIR}RivTernaryResultToTextureMapper.h
${CEE_CURRENT_LIST_DIR}RivTextureCoordsCreator.h
${CEE_CURRENT_LIST_DIR}RivTernaryScalarMapper.h
${CEE_CURRENT_LIST_DIR}RivTernaryTextureCoordsCreator.h
${CEE_CURRENT_LIST_DIR}RivTernaryScalarMapperEffectGenerator.h
${CEE_CURRENT_LIST_DIR}RivScalarMapperUtils.h
${CEE_CURRENT_LIST_DIR}RivCellEdgeGeometryUtils.h
${CEE_CURRENT_LIST_DIR}RivPipeQuadToSegmentMapper.h
${CEE_CURRENT_LIST_DIR}RivSingleCellPartGenerator.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RivCellEdgeEffectGenerator.cpp
${CEE_CURRENT_LIST_DIR}RivColorTableArray.cpp
${CEE_CURRENT_LIST_DIR}RivFaultPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivNNCGeometryGenerator.cpp
${CEE_CURRENT_LIST_DIR}RivFaultGeometryGenerator.cpp
${CEE_CURRENT_LIST_DIR}RivGridPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivTernarySaturationOverlayItem.cpp
${CEE_CURRENT_LIST_DIR}RivReservoirFaultsPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivReservoirPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivReservoirViewPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivPipeGeometryGenerator.cpp
${CEE_CURRENT_LIST_DIR}RivReservoirPipesPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivSourceInfo.cpp
${CEE_CURRENT_LIST_DIR}RivWellPathSourceInfo.cpp
${CEE_CURRENT_LIST_DIR}RivWellPathPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivWellPathCollectionPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivWellPipesPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivWellHeadPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivTextureCoordsCreator.cpp
${CEE_CURRENT_LIST_DIR}RivTernaryScalarMapper.cpp
${CEE_CURRENT_LIST_DIR}RivTernaryTextureCoordsCreator.cpp
${CEE_CURRENT_LIST_DIR}RivTernaryScalarMapperEffectGenerator.cpp
${CEE_CURRENT_LIST_DIR}RivScalarMapperUtils.cpp
${CEE_CURRENT_LIST_DIR}RivCellEdgeGeometryUtils.cpp
${CEE_CURRENT_LIST_DIR}RivPipeQuadToSegmentMapper.cpp
${CEE_CURRENT_LIST_DIR}RivSingleCellPartGenerator.cpp

)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "ModelVisualization" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
