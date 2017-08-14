
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RivCellEdgeEffectGenerator.h
${CEE_CURRENT_LIST_DIR}RivFaultPartMgr.h
${CEE_CURRENT_LIST_DIR}RivFaultGeometryGenerator.h
${CEE_CURRENT_LIST_DIR}RivNNCGeometryGenerator.h
${CEE_CURRENT_LIST_DIR}RivGridPartMgr.h
${CEE_CURRENT_LIST_DIR}RivTernarySaturationOverlayItem.h
${CEE_CURRENT_LIST_DIR}RivReservoirPartMgr.h
${CEE_CURRENT_LIST_DIR}RivReservoirViewPartMgr.h
${CEE_CURRENT_LIST_DIR}RivPipeGeometryGenerator.h
${CEE_CURRENT_LIST_DIR}RivReservoirFaultsPartMgr.h
${CEE_CURRENT_LIST_DIR}RivReservoirSimWellsPartMgr.h
${CEE_CURRENT_LIST_DIR}RivSourceInfo.h
${CEE_CURRENT_LIST_DIR}RivWellPathSourceInfo.h
${CEE_CURRENT_LIST_DIR}RivWellPathPartMgr.h
${CEE_CURRENT_LIST_DIR}RivSimWellPipesPartMgr.h
${CEE_CURRENT_LIST_DIR}RivWellHeadPartMgr.h
${CEE_CURRENT_LIST_DIR}RivResultToTextureMapper.h
${CEE_CURRENT_LIST_DIR}RivCompletionTypeResultToTextureMapper.h
${CEE_CURRENT_LIST_DIR}RivDefaultResultToTextureMapper.h
${CEE_CURRENT_LIST_DIR}RivTernaryResultToTextureMapper.h
${CEE_CURRENT_LIST_DIR}RivTextureCoordsCreator.h
${CEE_CURRENT_LIST_DIR}RivTernaryScalarMapper.h
${CEE_CURRENT_LIST_DIR}RivTernaryTextureCoordsCreator.h
${CEE_CURRENT_LIST_DIR}RivTernaryScalarMapperEffectGenerator.h
${CEE_CURRENT_LIST_DIR}RivScalarMapperUtils.h
${CEE_CURRENT_LIST_DIR}RivCellEdgeGeometryUtils.h
${CEE_CURRENT_LIST_DIR}RivPipeQuadToSegmentMapper.h
${CEE_CURRENT_LIST_DIR}RivSingleCellPartGenerator.h
${CEE_CURRENT_LIST_DIR}RivSimWellPipeSourceInfo.h
${CEE_CURRENT_LIST_DIR}RivWellSpheresPartMgr.h
${CEE_CURRENT_LIST_DIR}RivPartPriority.h
${CEE_CURRENT_LIST_DIR}RivObjectSourceInfo.h
${CEE_CURRENT_LIST_DIR}RivWellConnectionsPartMgr.h
${CEE_CURRENT_LIST_DIR}RivFishbonesSubsPartMgr.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RivCellEdgeEffectGenerator.cpp
${CEE_CURRENT_LIST_DIR}RivFaultPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivNNCGeometryGenerator.cpp
${CEE_CURRENT_LIST_DIR}RivFaultGeometryGenerator.cpp
${CEE_CURRENT_LIST_DIR}RivGridPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivTernarySaturationOverlayItem.cpp
${CEE_CURRENT_LIST_DIR}RivReservoirFaultsPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivReservoirPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivReservoirViewPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivPipeGeometryGenerator.cpp
${CEE_CURRENT_LIST_DIR}RivReservoirSimWellsPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivSourceInfo.cpp
${CEE_CURRENT_LIST_DIR}RivWellPathSourceInfo.cpp
${CEE_CURRENT_LIST_DIR}RivWellPathPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivSimWellPipesPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivWellHeadPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivTextureCoordsCreator.cpp
${CEE_CURRENT_LIST_DIR}RivTernaryScalarMapper.cpp
${CEE_CURRENT_LIST_DIR}RivTernaryTextureCoordsCreator.cpp
${CEE_CURRENT_LIST_DIR}RivTernaryScalarMapperEffectGenerator.cpp
${CEE_CURRENT_LIST_DIR}RivScalarMapperUtils.cpp
${CEE_CURRENT_LIST_DIR}RivCellEdgeGeometryUtils.cpp
${CEE_CURRENT_LIST_DIR}RivPipeQuadToSegmentMapper.cpp
${CEE_CURRENT_LIST_DIR}RivSingleCellPartGenerator.cpp
${CEE_CURRENT_LIST_DIR}RivSimWellPipeSourceInfo.cpp
${CEE_CURRENT_LIST_DIR}RivWellSpheresPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivObjectSourceInfo.cpp
${CEE_CURRENT_LIST_DIR}RivWellConnectionsPartMgr.cpp
${CEE_CURRENT_LIST_DIR}RivFishbonesSubsPartMgr.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "ModelVisualization" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
