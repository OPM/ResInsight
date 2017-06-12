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

#include "RicFishbonesTransmissibilityCalculationFeatureImp.h"

#include "RigEclipseCaseData.h"
#include "RicExportCompletionDataSettingsUi.h"
#include "RicWellPathExportCompletionDataFeature.h"
#include "RimWellPath.h"
#include "RigWellPath.h"
#include "RimFishboneWellPath.h"
#include "RimFishbonesCollection.h"
#include "RigMainGrid.h"
#include "RimFishbonesMultipleSubs.h"
#include "RimFishboneWellPathCollection.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> RicFishbonesTransmissibilityCalculationFeatureImp::generateFishboneLateralsCompdatValues(const RimWellPath* wellPath, const RicExportCompletionDataSettingsUi& settings)
{
    // Generate data
    const RigEclipseCaseData* caseData =  settings.caseToApply()->eclipseCaseData();
    std::vector<WellSegmentLocation> locations = RicWellPathExportCompletionDataFeature::findWellSegmentLocations(settings.caseToApply, wellPath);

    // Filter out cells where main bore is present
    if (settings.removeLateralsInMainBoreCells())
    {
        std::vector<size_t> wellPathCells = RicWellPathExportCompletionDataFeature::findIntersectingCells(caseData, wellPath->wellPathGeometry()->m_wellPathPoints);
        RicWellPathExportCompletionDataFeature::markWellPathCells(wellPathCells, &locations);
    }

    RigMainGrid* grid = settings.caseToApply->eclipseCaseData()->mainGrid();

    std::vector<RigCompletionData> completionData;

    for (const WellSegmentLocation& location : locations)
    {
        for (const WellSegmentLateral& lateral : location.laterals)
        {
            for (const WellSegmentLateralIntersection& intersection : lateral.intersections)
            {
                if (intersection.mainBoreCell && settings.removeLateralsInMainBoreCells()) continue;

                size_t i, j, k;
                grid->ijkFromCellIndex(intersection.cellIndex, &i, &j, &k);
                RigCompletionData completion(wellPath->name(), IJKCellIndex(i, j, k));
                completion.addMetadata(location.fishbonesSubs->name(), QString("Sub: %1 Lateral: %2").arg(location.subIndex).arg(lateral.lateralIndex));
                double diameter = location.fishbonesSubs->holeDiameter() / 1000;
                if (settings.computeTransmissibility())
                {
                    double transmissibility = RicWellPathExportCompletionDataFeature::calculateTransmissibility(settings.caseToApply,
                                                                        wellPath,
                                                                        intersection.lengthsInCell,
                                                                        location.fishbonesSubs->skinFactor(),
                                                                        diameter / 2,
                                                                        intersection.cellIndex);
                    completion.setFromFishbone(transmissibility, location.fishbonesSubs->skinFactor());
                }
                else {
                    CellDirection direction = RicWellPathExportCompletionDataFeature::calculateDirectionInCell(settings.caseToApply, intersection.cellIndex, intersection.lengthsInCell);
                    completion.setFromFishbone(diameter, direction);
                }
                completionData.push_back(completion);
            }
        }
    }

    return completionData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RigCompletionData> RicFishbonesTransmissibilityCalculationFeatureImp::generateFishbonesImportedLateralsCompdatValues(const RimWellPath* wellPath, const RicExportCompletionDataSettingsUi& settings)
{
    std::vector<RigCompletionData> completionData;

    std::vector<size_t> wellPathCells = RicWellPathExportCompletionDataFeature::findIntersectingCells(settings.caseToApply()->eclipseCaseData(), wellPath->wellPathGeometry()->m_wellPathPoints);

    double diameter = wellPath->fishbonesCollection()->wellPathCollection()->holeDiameter() / 1000;
    for (const RimFishboneWellPath* fishbonesPath : wellPath->fishbonesCollection()->wellPathCollection()->wellPaths())
    {
        std::vector<WellPathCellIntersectionInfo> intersectedCells = RigWellPathIntersectionTools::findCellsIntersectedByPath(settings.caseToApply->eclipseCaseData(), fishbonesPath->coordinates());
        for (auto& cell : intersectedCells)
        {
            if (std::find(wellPathCells.begin(), wellPathCells.end(), cell.cellIndex) != wellPathCells.end()) continue;

            size_t i, j, k;
            settings.caseToApply->eclipseCaseData()->mainGrid()->ijkFromCellIndex(cell.cellIndex, &i, &j, &k);
            RigCompletionData completion(wellPath->name(), IJKCellIndex(i, j, k));
            completion.addMetadata(fishbonesPath->name(), "");
            if (settings.computeTransmissibility())
            {
                double skinFactor = wellPath->fishbonesCollection()->wellPathCollection()->skinFactor();
                double transmissibility = RicWellPathExportCompletionDataFeature::calculateTransmissibility(settings.caseToApply(),
                                                                    wellPath,
                                                                    cell.internalCellLengths,
                                                                    skinFactor,
                                                                    diameter / 2,
                                                                    cell.cellIndex);
                completion.setFromFishbone(transmissibility, skinFactor);
            }
            else {
                CellDirection direction = RicWellPathExportCompletionDataFeature::calculateDirectionInCell(settings.caseToApply, cell.cellIndex, cell.internalCellLengths);
                completion.setFromFishbone(diameter, direction);
            }
            completionData.push_back(completion);
        }
    }

    return completionData;
}


