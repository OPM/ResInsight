set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RigAccWellFlowCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RigEclipseWellLogExtractor.h
    ${CMAKE_CURRENT_LIST_DIR}/RigFishbonesGeometry.h
    ${CMAKE_CURRENT_LIST_DIR}/RigGeoMechWellLogExtractor.h
    ${CMAKE_CURRENT_LIST_DIR}/RigMswCenterLineCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RigOsduWellLogData.h
    ${CMAKE_CURRENT_LIST_DIR}/RigPipeInCellEvaluator.h
    ${CMAKE_CURRENT_LIST_DIR}/RigSimulationWellCenterLineCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RigSimulationWellCoordsAndMD.h
    ${CMAKE_CURRENT_LIST_DIR}/RigSimWellData.h
    ${CMAKE_CURRENT_LIST_DIR}/RigWellAllocationOverTime.h
    ${CMAKE_CURRENT_LIST_DIR}/RigWellDiskData.h
    ${CMAKE_CURRENT_LIST_DIR}/RigWellLogCurveData.h
    ${CMAKE_CURRENT_LIST_DIR}/RigWellLogExtractionTools.h
    ${CMAKE_CURRENT_LIST_DIR}/RigWellLogExtractor.h
    ${CMAKE_CURRENT_LIST_DIR}/RigWellLogIndexDepthOffset.h
    ${CMAKE_CURRENT_LIST_DIR}/RigWellPath.h
    ${CMAKE_CURRENT_LIST_DIR}/RigWellPathFormations.h
    ${CMAKE_CURRENT_LIST_DIR}/RigWellPathGeometryExporter.h
    ${CMAKE_CURRENT_LIST_DIR}/RigWellPathGeometryTools.h
    ${CMAKE_CURRENT_LIST_DIR}/RigWellPathIntersectionTools.h
    ${CMAKE_CURRENT_LIST_DIR}/RigWellResultBranch.h
    ${CMAKE_CURRENT_LIST_DIR}/RigWellResultFrame.h
    ${CMAKE_CURRENT_LIST_DIR}/RigWellResultPoint.h
    ${CMAKE_CURRENT_LIST_DIR}/RigWellTargetMapping.h
    ${CMAKE_CURRENT_LIST_DIR}/RigWellLogData.h
    ${CMAKE_CURRENT_LIST_DIR}/RigWellLogLasFile.h
    ${CMAKE_CURRENT_LIST_DIR}/RigWellLogCsvFile.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RigAccWellFlowCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigEclipseWellLogExtractor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigFishbonesGeometry.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigGeoMechWellLogExtractor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigMswCenterLineCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigOsduWellLogData.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigSimulationWellCenterLineCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigSimulationWellCoordsAndMD.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigSimWellData.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigWellAllocationOverTime.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigWellDiskData.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigWellLogCurveData.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigWellLogExtractor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigWellLogIndexDepthOffset.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigWellPath.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigWellPathFormations.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigWellPathGeometryExporter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigWellPathGeometryTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigWellPathIntersectionTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigWellResultBranch.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigWellResultFrame.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigWellResultPoint.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigWellTargetMapping.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigWellLogData.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigWellLogLasFile.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigWellLogCsvFile.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
