/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimCompletionCellIntersectionCalc.h"

#include "RiaDefines.h"
#include "RimProject.h"
#include "RimOilField.h"
#include "RimWellPathCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCompletions.h"
#include "RimFishbonesCollection.h"
#include "RimFishbonesMultipleSubs.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"

#include "RigMainGrid.h"
#include "RigWellPath.h"
#include "RigWellPathIntersectionTools.h"

#include <QDateTime>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCompletionCellIntersectionCalc::calculateIntersections(const RimProject* project, const RigMainGrid* grid, std::vector<double>& values, const QDateTime& fromDate)
{
    for (const RimWellPath* wellPath : project->activeOilField()->wellPathCollection->wellPaths)
    {
        if (wellPath->showWellPath())
        {
            calculateWellPathIntersections(wellPath, grid, values, fromDate);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCompletionCellIntersectionCalc::calculateWellPathIntersections(const RimWellPath* wellPath, const RigMainGrid* grid, std::vector<double>& values, const QDateTime& fromDate)
{
    std::vector<HexIntersectionInfo> intersections = RigWellPathIntersectionTools::getIntersectedCells(grid, wellPath->wellPathGeometry()->m_wellPathPoints);
    
    for (auto& intersection : intersections)
    {
        values[intersection.m_hexIndex] = RiaDefines::WELL_PATH;
    }

    for (const RimFishbonesMultipleSubs* fishbones : wellPath->fishbonesCollection()->fishbonesSubs)
    {
        calculateFishbonesIntersections(fishbones, grid, values);
    }

    for (const RimPerforationInterval* perforationInterval : wellPath->perforationIntervalCollection()->perforations())
    {
        if (perforationInterval->isActiveOnDate(fromDate))
        {
            calculatePerforationIntersections(wellPath, perforationInterval, grid, values);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCompletionCellIntersectionCalc::calculateFishbonesIntersections(const RimFishbonesMultipleSubs* fishbonesSubs, const RigMainGrid* grid, std::vector<double>& values)
{
    for (auto& sub : fishbonesSubs->installedLateralIndices())
    {
        for (size_t lateralIndex : sub.lateralIndices)
        {
            std::vector<HexIntersectionInfo> intersections = RigWellPathIntersectionTools::getIntersectedCells(grid, fishbonesSubs->coordsForLateral(sub.subIndex, lateralIndex));
            for (auto& intersection : intersections)
            {
                values[intersection.m_hexIndex] = RiaDefines::FISHBONES;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCompletionCellIntersectionCalc::calculatePerforationIntersections(const RimWellPath* wellPath, const RimPerforationInterval* perforationInterval, const RigMainGrid* grid, std::vector<double>& values)
{
    std::vector<HexIntersectionInfo> intersections = RigWellPathIntersectionTools::getIntersectedCells(grid, wellPath->wellPathGeometry()->clippedPointSubset(perforationInterval->startMD(), perforationInterval->endMD()));
    for (auto& intersection : intersections)
    {
        values[intersection.m_hexIndex] = RiaDefines::PERFORATION_INTERVAL;
    }
}
