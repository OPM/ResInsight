/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "RigCompletionData.h"

#include "RicExportCompletionDataSettingsUi.h"

#include "cvfBase.h"
#include "cvfVector3.h"

#include <vector>

class RigCell;
class RigEclipseCaseData;
class RigMainGrid;
class RimEclipseCase;
class RimFishbonesMultipleSubs;
class RimSimWellInView;
class RimWellPath;
class RifEclipseDataTableFormatter;
class RigVirtualPerforationTransmissibilities;

//==================================================================================================
/// 
//==================================================================================================
struct WellSegmentLateralIntersection
{
    WellSegmentLateralIntersection(int               segmentNumber,
                                   int               attachedSegmentNumber,
                                   size_t            globalCellIndex,
                                   double            length,
                                   double            depth,
                                   const cvf::Vec3d& lengthsInCell)
        : segmentNumber(segmentNumber)
        , attachedSegmentNumber(attachedSegmentNumber)
        , globalCellIndex(globalCellIndex)
        , mdFromPreviousIntersection(length)
        , tvdChangeFromPreviousIntersection(depth)
        , lengthsInCell(lengthsInCell)
        , mainBoreCell(false)
    {
    }

    int        segmentNumber;
    int        attachedSegmentNumber;
    size_t     globalCellIndex;
    bool       mainBoreCell;
    double     mdFromPreviousIntersection;
    double     tvdChangeFromPreviousIntersection;
    cvf::Vec3d lengthsInCell;
};

//==================================================================================================
/// 
//==================================================================================================
struct WellSegmentLateral
{
    WellSegmentLateral(size_t lateralIndex)
        : lateralIndex(lateralIndex)
        , branchNumber(0)
    {
    }

    size_t                                      lateralIndex;
    int                                         branchNumber;
    std::vector<WellSegmentLateralIntersection> intersections;
};

//==================================================================================================
/// 
//==================================================================================================
struct WellSegmentLocation
{
    WellSegmentLocation(const RimFishbonesMultipleSubs* subs,
                        double                          measuredDepth,
                        double                          trueVerticalDepth,
                        size_t                          subIndex,
                        int                             segmentNumber = -1)
        : fishbonesSubs(subs)
        , measuredDepth(measuredDepth)
        , trueVerticalDepth(trueVerticalDepth)
        , subIndex(subIndex)
        , segmentNumber(segmentNumber)
        , icdBranchNumber(-1)
        , icdSegmentNumber(-1)
    {
    }

    const RimFishbonesMultipleSubs* fishbonesSubs;
    double                          measuredDepth;
    double                          trueVerticalDepth;
    size_t                          subIndex;
    int                             segmentNumber;
    int                             icdBranchNumber;
    int                             icdSegmentNumber;
    std::vector<WellSegmentLateral> laterals;
};

//==================================================================================================
/// 
//==================================================================================================
class RicWellPathExportCompletionDataFeatureImpl
{

public:
    static std::vector<WellSegmentLocation>      findWellSegmentLocations(const RimEclipseCase* caseToApply, 
                                                                          const RimWellPath* wellPath);

    static std::vector<WellSegmentLocation>      findWellSegmentLocations(const RimEclipseCase* caseToApply, 
                                                                          const RimWellPath* wellPath, 
                                                                          const std::vector<RimFishbonesMultipleSubs*>& fishbonesSubs);

    static CellDirection                         calculateDirectionInCell(RimEclipseCase* eclipseCase, 
                                                                          size_t globalCellIndex, 
                                                                          const cvf::Vec3d& lengthsInCell);
    
    static double                                calculateTransmissibility(RimEclipseCase* eclipseCase, 
                                                                           const RimWellPath* wellPath, 
                                                                           const cvf::Vec3d& internalCellLengths, 
                                                                           double skinFactor, 
                                                                           double wellRadius, 
                                                                           size_t globalCellIndex, 
                                                                           bool useLateralNTG, 
                                                                           size_t volumeScaleConstant = 1, 
                                                                           CellDirection directionForVolumeScaling = CellDirection::DIR_I);


    static void                                  exportCompletions(const std::vector<RimWellPath*>& wellPaths, 
                                                                   const std::vector<RimSimWellInView*>& simWells, 
                                                                   const RicExportCompletionDataSettingsUi& exportSettings);

    static std::vector<RigCompletionData>        computeStaticCompletionsForWellPath(RimWellPath* wellPath,
                                                                                     RimEclipseCase* eclipseCase);

    static std::vector<RigCompletionData>        computeDynamicCompletionsForWellPath(RimWellPath* wellPath,
                                                                                      RimEclipseCase* eclipseCase,
                                                                                      size_t timeStepIndex);

private:
    static double                                calculateTransmissibilityAsEclipseDoes(RimEclipseCase* eclipseCase,
                                                                                        double skinFactor,
                                                                                        double wellRadius,
                                                                                        size_t globalCellIndex,
                                                                                        CellDirection direction);
    
    static RigCompletionData                     combineEclipseCellCompletions(const std::vector<RigCompletionData>& completions, 
                                                                               const RicExportCompletionDataSettingsUi& settings);

    static void                                  sortAndExportCompletionsToFile(const QString& exportFolder, 
                                                                         const QString& fileName, 
                                                                         std::vector<RigCompletionData>& completions, 
                                                                         RicExportCompletionDataSettingsUi::CompdatExportType exportType);

    static void                                  exportCompdatAndWpimultTables(const QString& folderName, 
                                                                        const QString& fileName, 
                                                                        const std::map<QString, std::vector<RigCompletionData>>& completionsPerGrid, 
                                                                        RicExportCompletionDataSettingsUi::CompdatExportType exportType);

    static std::vector<RigCompletionData>        getCompletionsForWellAndCompletionType(const std::vector<RigCompletionData>& completions,
                                                                                        const QString& wellName, 
                                                                                        RigCompletionData::CompletionType completionType);

    static std::map<RigCompletionDataGridCell, std::vector<RigCompletionData> > 
      getCompletionsForWell(const std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>>& cellToCompletionMap, 
                            const QString& wellName);

    static void                                  exportCompdatTableUsingFormatter(RifEclipseDataTableFormatter& formatter, 
                                                                      const QString& gridName, 
                                                                      const std::vector<RigCompletionData>& completionData);

    static void                                 exportWpimultTableUsingFormatter(RifEclipseDataTableFormatter& formatter,
                                                                     const QString& gridName,
                                                                     const std::vector<RigCompletionData>& completionData);

    static std::vector<RigCompletionData>       generatePerforationsCompdatValues(const RimWellPath* wellPath,
                                                                                  const RicExportCompletionDataSettingsUi& settings);

    static bool                                 wellSegmentLocationOrdering(const WellSegmentLocation& first,
                                                                            const WellSegmentLocation& second);

    static void                                 assignLateralIntersections(const RimEclipseCase* caseToApply,
                                                                           WellSegmentLocation* location,
                                                                           int* branchNum,
                                                                           int* segmentNum);

    static void                                 assignLateralIntersectionsAndBranchAndSegmentNumbers(const RimEclipseCase* caseToApply,
                                                                                                     std::vector<WellSegmentLocation>* locations);

    static void                                 appendCompletionData(std::map<RigCompletionDataGridCell, std::vector<RigCompletionData>>* completionData,
                                                                     const std::vector<RigCompletionData>& data);
};
