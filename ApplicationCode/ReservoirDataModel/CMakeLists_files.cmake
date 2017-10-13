
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RigActiveCellInfo.h
${CEE_CURRENT_LIST_DIR}RigCell.h
${CEE_CURRENT_LIST_DIR}RigEclipseCaseData.h
${CEE_CURRENT_LIST_DIR}RigGridBase.h
${CEE_CURRENT_LIST_DIR}RigGridManager.h
${CEE_CURRENT_LIST_DIR}RigResultAccessor.h
${CEE_CURRENT_LIST_DIR}RigResultAccessorFactory.h
${CEE_CURRENT_LIST_DIR}RigAllGridCellsResultAccessor.h
${CEE_CURRENT_LIST_DIR}RigActiveCellsResultAccessor.h
${CEE_CURRENT_LIST_DIR}RigCellEdgeResultAccessor.h
${CEE_CURRENT_LIST_DIR}RigCellGeometryTools.h
${CEE_CURRENT_LIST_DIR}RigCombTransResultAccessor.h
${CEE_CURRENT_LIST_DIR}RigCombMultResultAccessor.h
${CEE_CURRENT_LIST_DIR}RigCompletionData.h
${CEE_CURRENT_LIST_DIR}RigResultModifier.h
${CEE_CURRENT_LIST_DIR}RigResultModifierFactory.h
${CEE_CURRENT_LIST_DIR}RigFormationNames.h
${CEE_CURRENT_LIST_DIR}RigFlowDiagResultAddress.h
${CEE_CURRENT_LIST_DIR}RigFlowDiagResults.h
${CEE_CURRENT_LIST_DIR}RigFlowDiagResultFrames.h
${CEE_CURRENT_LIST_DIR}RigFlowDiagSolverInterface.h
${CEE_CURRENT_LIST_DIR}RigFlowDiagInterfaceTools.h
${CEE_CURRENT_LIST_DIR}RigFlowDiagStatCalc.h
${CEE_CURRENT_LIST_DIR}RigFlowDiagVisibleCellsStatCalc.h
${CEE_CURRENT_LIST_DIR}RigAccWellFlowCalculator.h
${CEE_CURRENT_LIST_DIR}RigWellLogExtractor.h
${CEE_CURRENT_LIST_DIR}RigEclipseWellLogExtractor.h
${CEE_CURRENT_LIST_DIR}RigLocalGrid.h
${CEE_CURRENT_LIST_DIR}RigMainGrid.h
${CEE_CURRENT_LIST_DIR}RigReservoirBuilderMock.h
${CEE_CURRENT_LIST_DIR}RigCaseCellResultsData.h
${CEE_CURRENT_LIST_DIR}RigSimWellData.h
${CEE_CURRENT_LIST_DIR}RigWellPath.h
${CEE_CURRENT_LIST_DIR}RigFault.h
${CEE_CURRENT_LIST_DIR}RigNNCData.h
${CEE_CURRENT_LIST_DIR}cvfGeometryTools.h
${CEE_CURRENT_LIST_DIR}cvfGeometryTools.inl
${CEE_CURRENT_LIST_DIR}RigPipeInCellEvaluator.h
${CEE_CURRENT_LIST_DIR}RigTernaryResultAccessor2d.h
${CEE_CURRENT_LIST_DIR}RigEclipseNativeStatCalc.h
${CEE_CURRENT_LIST_DIR}RigEclipseNativeVisibleCellsStatCalc.h
${CEE_CURRENT_LIST_DIR}RigEclipseMultiPropertyStatCalc.h
${CEE_CURRENT_LIST_DIR}RigWellLogCurveData.h
${CEE_CURRENT_LIST_DIR}RigWellLogExtractionTools.h
${CEE_CURRENT_LIST_DIR}RigHexIntersectionTools.h
${CEE_CURRENT_LIST_DIR}RigTimeHistoryResultAccessor.h
${CEE_CURRENT_LIST_DIR}RigCurveDataTools.h
${CEE_CURRENT_LIST_DIR}RigObservedData.h
${CEE_CURRENT_LIST_DIR}RigLasFileExporter.h
${CEE_CURRENT_LIST_DIR}RigSimulationWellCoordsAndMD.h
${CEE_CURRENT_LIST_DIR}RigFishbonesGeometry.h
${CEE_CURRENT_LIST_DIR}RigTesselatorTools.h
${CEE_CURRENT_LIST_DIR}RigCellGeometryTools.h
${CEE_CURRENT_LIST_DIR}RigWellPathIntersectionTools.h
${CEE_CURRENT_LIST_DIR}RigEclipseResultInfo.h
${CEE_CURRENT_LIST_DIR}RigTofAccumulatedPhaseFractionsCalculator.h
${CEE_CURRENT_LIST_DIR}RigTransmissibilityEquations.h
${CEE_CURRENT_LIST_DIR}RigNumberOfFloodedPoreVolumesCalculator.h

)

if (RESINSIGHT_ENABLE_PROTOTYPE_FEATURE_FRACTURES)
    list (APPEND SOURCE_GROUP_HEADER_FILES
        ${CEE_CURRENT_LIST_DIR}RigEclipseToStimPlanCellTransmissibilityCalculator.h
        ${CEE_CURRENT_LIST_DIR}RigTransmissibilityCondenser.h
        ${CEE_CURRENT_LIST_DIR}RigFractureTransmissibilityEquations.h
        ${CEE_CURRENT_LIST_DIR}RigStimPlanFractureDefinition.h
        ${CEE_CURRENT_LIST_DIR}RigFractureGrid.h
        ${CEE_CURRENT_LIST_DIR}RigFractureCell.h
        ${CEE_CURRENT_LIST_DIR}RigWellPathStimplanIntersector.h
    )
endif()


set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RigActiveCellInfo.cpp
${CEE_CURRENT_LIST_DIR}RigCell.cpp
${CEE_CURRENT_LIST_DIR}RigEclipseCaseData.cpp
${CEE_CURRENT_LIST_DIR}RigGridBase.cpp
${CEE_CURRENT_LIST_DIR}RigGridManager.cpp
${CEE_CURRENT_LIST_DIR}RigResultAccessor.cpp
${CEE_CURRENT_LIST_DIR}RigResultAccessorFactory.cpp
${CEE_CURRENT_LIST_DIR}RigAllGridCellsResultAccessor.cpp
${CEE_CURRENT_LIST_DIR}RigActiveCellsResultAccessor.cpp
${CEE_CURRENT_LIST_DIR}RigCellEdgeResultAccessor.cpp
${CEE_CURRENT_LIST_DIR}RigCellGeometryTools.cpp
${CEE_CURRENT_LIST_DIR}RigCombTransResultAccessor.cpp
${CEE_CURRENT_LIST_DIR}RigCombMultResultAccessor.cpp
${CEE_CURRENT_LIST_DIR}RigCompletionData.cpp
${CEE_CURRENT_LIST_DIR}RigResultModifierFactory.cpp
${CEE_CURRENT_LIST_DIR}RigFormationNames.cpp
${CEE_CURRENT_LIST_DIR}RigFlowDiagResultAddress.cpp
${CEE_CURRENT_LIST_DIR}RigFlowDiagResults.cpp
${CEE_CURRENT_LIST_DIR}RigFlowDiagResultFrames.cpp
${CEE_CURRENT_LIST_DIR}RigFlowDiagSolverInterface.cpp
${CEE_CURRENT_LIST_DIR}RigFlowDiagStatCalc.cpp
${CEE_CURRENT_LIST_DIR}RigFlowDiagVisibleCellsStatCalc.cpp
${CEE_CURRENT_LIST_DIR}RigAccWellFlowCalculator.cpp
${CEE_CURRENT_LIST_DIR}RigWellLogExtractor.cpp
${CEE_CURRENT_LIST_DIR}RigEclipseWellLogExtractor.cpp
${CEE_CURRENT_LIST_DIR}RigLocalGrid.cpp
${CEE_CURRENT_LIST_DIR}RigMainGrid.cpp
${CEE_CURRENT_LIST_DIR}RigReservoirBuilderMock.cpp
${CEE_CURRENT_LIST_DIR}RigCaseCellResultsData.cpp
${CEE_CURRENT_LIST_DIR}RigSimWellData.cpp
${CEE_CURRENT_LIST_DIR}RigWellPath.cpp
${CEE_CURRENT_LIST_DIR}RigFault.cpp
${CEE_CURRENT_LIST_DIR}RigNNCData.cpp
${CEE_CURRENT_LIST_DIR}cvfGeometryTools.cpp
${CEE_CURRENT_LIST_DIR}RigTernaryResultAccessor2d.cpp
${CEE_CURRENT_LIST_DIR}RigEclipseNativeStatCalc.cpp
${CEE_CURRENT_LIST_DIR}RigEclipseNativeVisibleCellsStatCalc.cpp
${CEE_CURRENT_LIST_DIR}RigEclipseMultiPropertyStatCalc.cpp
${CEE_CURRENT_LIST_DIR}RigWellLogCurveData.cpp
${CEE_CURRENT_LIST_DIR}RigHexIntersectionTools.cpp
${CEE_CURRENT_LIST_DIR}RigTimeHistoryResultAccessor.cpp
${CEE_CURRENT_LIST_DIR}RigCurveDataTools.cpp    
${CEE_CURRENT_LIST_DIR}RigObservedData.cpp
${CEE_CURRENT_LIST_DIR}RigLasFileExporter.cpp
${CEE_CURRENT_LIST_DIR}RigSimulationWellCoordsAndMD.cpp
${CEE_CURRENT_LIST_DIR}RigFishbonesGeometry.cpp
${CEE_CURRENT_LIST_DIR}RigTesselatorTools.cpp
${CEE_CURRENT_LIST_DIR}RigCellGeometryTools.cpp
${CEE_CURRENT_LIST_DIR}RigWellPathIntersectionTools.cpp
${CEE_CURRENT_LIST_DIR}RigEclipseResultInfo.cpp
${CEE_CURRENT_LIST_DIR}RigTofAccumulatedPhaseFractionsCalculator.cpp
${CEE_CURRENT_LIST_DIR}RigTransmissibilityEquations.cpp
${CEE_CURRENT_LIST_DIR}RigNumberOfFloodedPoreVolumesCalculator.cpp
)

if (RESINSIGHT_ENABLE_PROTOTYPE_FEATURE_FRACTURES)
    list (APPEND SOURCE_GROUP_SOURCE_FILES
        ${CEE_CURRENT_LIST_DIR}RigEclipseToStimPlanCellTransmissibilityCalculator.cpp
        ${CEE_CURRENT_LIST_DIR}RigTransmissibilityCondenser.cpp
        ${CEE_CURRENT_LIST_DIR}RigFractureTransmissibilityEquations.cpp
        ${CEE_CURRENT_LIST_DIR}RigStimPlanFractureDefinition.cpp
        ${CEE_CURRENT_LIST_DIR}RigFractureGrid.cpp
        ${CEE_CURRENT_LIST_DIR}RigFractureCell.cpp
        ${CEE_CURRENT_LIST_DIR}RigWellPathStimplanIntersector.cpp
    )
endif()


list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "ReservoirDataModel" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
