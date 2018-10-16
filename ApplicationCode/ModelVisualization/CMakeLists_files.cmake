
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RivCellEdgeEffectGenerator.h
${CMAKE_CURRENT_LIST_DIR}/RivFaultPartMgr.h
${CMAKE_CURRENT_LIST_DIR}/RivFaultGeometryGenerator.h
${CMAKE_CURRENT_LIST_DIR}/RivNNCGeometryGenerator.h
${CMAKE_CURRENT_LIST_DIR}/RivGridPartMgr.h
${CMAKE_CURRENT_LIST_DIR}/RivTernarySaturationOverlayItem.h
${CMAKE_CURRENT_LIST_DIR}/RivReservoirPartMgr.h
${CMAKE_CURRENT_LIST_DIR}/RivReservoirViewPartMgr.h
${CMAKE_CURRENT_LIST_DIR}/RivPipeGeometryGenerator.h
${CMAKE_CURRENT_LIST_DIR}/RivReservoirFaultsPartMgr.h
${CMAKE_CURRENT_LIST_DIR}/RivReservoirSimWellsPartMgr.h
${CMAKE_CURRENT_LIST_DIR}/RivSourceInfo.h
${CMAKE_CURRENT_LIST_DIR}/RivWellPathSourceInfo.h
${CMAKE_CURRENT_LIST_DIR}/RivWellPathPartMgr.h
${CMAKE_CURRENT_LIST_DIR}/RivWellPathsPartMgr.h
${CMAKE_CURRENT_LIST_DIR}/RivSimWellPipesPartMgr.h
${CMAKE_CURRENT_LIST_DIR}/RivWellHeadPartMgr.h
${CMAKE_CURRENT_LIST_DIR}/RivResultToTextureMapper.h
${CMAKE_CURRENT_LIST_DIR}/RivCompletionTypeResultToTextureMapper.h
${CMAKE_CURRENT_LIST_DIR}/RivDefaultResultToTextureMapper.h
${CMAKE_CURRENT_LIST_DIR}/RivTernaryResultToTextureMapper.h
${CMAKE_CURRENT_LIST_DIR}/RivTextureCoordsCreator.h
${CMAKE_CURRENT_LIST_DIR}/RivTernaryScalarMapper.h
${CMAKE_CURRENT_LIST_DIR}/RivTernaryTextureCoordsCreator.h
${CMAKE_CURRENT_LIST_DIR}/RivTernaryScalarMapperEffectGenerator.h
${CMAKE_CURRENT_LIST_DIR}/RivScalarMapperUtils.h
${CMAKE_CURRENT_LIST_DIR}/RivCellEdgeGeometryUtils.h
${CMAKE_CURRENT_LIST_DIR}/RivPipeQuadToSegmentMapper.h
${CMAKE_CURRENT_LIST_DIR}/RivSingleCellPartGenerator.h
${CMAKE_CURRENT_LIST_DIR}/RivSimWellPipeSourceInfo.h
${CMAKE_CURRENT_LIST_DIR}/RivWellSpheresPartMgr.h
${CMAKE_CURRENT_LIST_DIR}/RivPartPriority.h
${CMAKE_CURRENT_LIST_DIR}/RivObjectSourceInfo.h
${CMAKE_CURRENT_LIST_DIR}/RivWellConnectionsPartMgr.h
${CMAKE_CURRENT_LIST_DIR}/RivFishbonesSubsPartMgr.h
${CMAKE_CURRENT_LIST_DIR}/RivTensorResultPartMgr.h
${CMAKE_CURRENT_LIST_DIR}/RivWellFracturePartMgr.h
${CMAKE_CURRENT_LIST_DIR}/Riv3dWellLogPlanePartMgr.h
${CMAKE_CURRENT_LIST_DIR}/Riv3dWellLogCurveGeometryGenerator.h
${CMAKE_CURRENT_LIST_DIR}/RivWellConnectionFactorPartMgr.h
${CMAKE_CURRENT_LIST_DIR}/RivWellConnectionFactorGeometryGenerator.h
${CMAKE_CURRENT_LIST_DIR}/RivWellConnectionSourceInfo.h
${CMAKE_CURRENT_LIST_DIR}/RivSimWellConnectionSourceInfo.h
${CMAKE_CURRENT_LIST_DIR}/Riv3dWellLogDrawSurfaceGenerator.h
${CMAKE_CURRENT_LIST_DIR}/RivMeshLinesSourceInfo.h
${CMAKE_CURRENT_LIST_DIR}/Riv2dGridProjectionPartMgr.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RivCellEdgeEffectGenerator.cpp
${CMAKE_CURRENT_LIST_DIR}/RivFaultPartMgr.cpp
${CMAKE_CURRENT_LIST_DIR}/RivNNCGeometryGenerator.cpp
${CMAKE_CURRENT_LIST_DIR}/RivFaultGeometryGenerator.cpp
${CMAKE_CURRENT_LIST_DIR}/RivGridPartMgr.cpp
${CMAKE_CURRENT_LIST_DIR}/RivTernarySaturationOverlayItem.cpp
${CMAKE_CURRENT_LIST_DIR}/RivReservoirFaultsPartMgr.cpp
${CMAKE_CURRENT_LIST_DIR}/RivReservoirPartMgr.cpp
${CMAKE_CURRENT_LIST_DIR}/RivReservoirViewPartMgr.cpp
${CMAKE_CURRENT_LIST_DIR}/RivPipeGeometryGenerator.cpp
${CMAKE_CURRENT_LIST_DIR}/RivReservoirSimWellsPartMgr.cpp
${CMAKE_CURRENT_LIST_DIR}/RivSourceInfo.cpp
${CMAKE_CURRENT_LIST_DIR}/RivWellPathSourceInfo.cpp
${CMAKE_CURRENT_LIST_DIR}/RivWellPathPartMgr.cpp
${CMAKE_CURRENT_LIST_DIR}/RivWellPathsPartMgr.cpp
${CMAKE_CURRENT_LIST_DIR}/RivSimWellPipesPartMgr.cpp
${CMAKE_CURRENT_LIST_DIR}/RivWellHeadPartMgr.cpp
${CMAKE_CURRENT_LIST_DIR}/RivTextureCoordsCreator.cpp
${CMAKE_CURRENT_LIST_DIR}/RivTernaryScalarMapper.cpp
${CMAKE_CURRENT_LIST_DIR}/RivTernaryTextureCoordsCreator.cpp
${CMAKE_CURRENT_LIST_DIR}/RivTernaryScalarMapperEffectGenerator.cpp
${CMAKE_CURRENT_LIST_DIR}/RivScalarMapperUtils.cpp
${CMAKE_CURRENT_LIST_DIR}/RivCellEdgeGeometryUtils.cpp
${CMAKE_CURRENT_LIST_DIR}/RivPipeQuadToSegmentMapper.cpp
${CMAKE_CURRENT_LIST_DIR}/RivSingleCellPartGenerator.cpp
${CMAKE_CURRENT_LIST_DIR}/RivSimWellPipeSourceInfo.cpp
${CMAKE_CURRENT_LIST_DIR}/RivWellSpheresPartMgr.cpp
${CMAKE_CURRENT_LIST_DIR}/RivObjectSourceInfo.cpp
${CMAKE_CURRENT_LIST_DIR}/RivWellConnectionsPartMgr.cpp
${CMAKE_CURRENT_LIST_DIR}/RivFishbonesSubsPartMgr.cpp
${CMAKE_CURRENT_LIST_DIR}/RivTensorResultPartMgr.cpp
${CMAKE_CURRENT_LIST_DIR}/RivWellFracturePartMgr.cpp
${CMAKE_CURRENT_LIST_DIR}/Riv3dWellLogPlanePartMgr.cpp
${CMAKE_CURRENT_LIST_DIR}/Riv3dWellLogCurveGeometryGenerator.cpp
${CMAKE_CURRENT_LIST_DIR}/RivWellConnectionFactorPartMgr.cpp
${CMAKE_CURRENT_LIST_DIR}/RivWellConnectionFactorGeometryGenerator.cpp
${CMAKE_CURRENT_LIST_DIR}/RivWellConnectionSourceInfo.cpp
${CMAKE_CURRENT_LIST_DIR}/RivSimWellConnectionSourceInfo.cpp
${CMAKE_CURRENT_LIST_DIR}/Riv3dWellLogDrawSurfaceGenerator.cpp
${CMAKE_CURRENT_LIST_DIR}/RivMeshLinesSourceInfo.cpp
${CMAKE_CURRENT_LIST_DIR}/Riv2dGridProjectionPartMgr.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "ModelVisualization" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
