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

#include "RigCellGeometryTools.h"
#include "RigEclipseCaseData.h"
#include "RigFractureCell.h"
#include "RigFractureGrid.h"
#include "RigHexIntersectionTools.h"
#include "RigMainGrid.h"
#include "RigWellPath.h"
#include "RigWellPathIntersectionTools.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimFishbonesCollection.h"
#include "RimFishbonesMultipleSubs.h"
#include "RimFracture.h"
#include "RimFractureTemplate.h"
#include "RimOilField.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimProject.h"
#include "RimSimWellFracture.h"
#include "RimSimWellFractureCollection.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathCompletions.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"

#include <QDateTime>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCompletionCellIntersectionCalc::calculateCompletionTypeResult(const RimProject*     project,
                                                                      const RimEclipseCase* eclipseCase,
                                                                      std::vector<double>&  completionTypeCellResults,
                                                                      const QDateTime&      fromDate)
{
    CVF_ASSERT(eclipseCase && eclipseCase->eclipseCaseData());

    const RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();

    if (project->activeOilField()->wellPathCollection->isActive)
    {
        for (const RimWellPath* wellPath : project->activeOilField()->wellPathCollection->wellPaths)
        {
            if (wellPath->showWellPath())
            {
                calculateWellPathIntersections(wellPath, eclipseCaseData, completionTypeCellResults, fromDate);
            }
        }
    }

    // NOTE : Never compute completion type result for simulation well fractures, as these are defined per view
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCompletionCellIntersectionCalc::calculateWellPathIntersections(const RimWellPath*        wellPath,
                                                                       const RigEclipseCaseData* eclipseCaseData,
                                                                       std::vector<double>&      values,
                                                                       const QDateTime&          fromDate)
{
    if (wellPath->wellPathGeometry())
    {
        auto intersectedCells = RigWellPathIntersectionTools::findIntersectedGlobalCellIndices(
            eclipseCaseData, wellPath->wellPathGeometry()->m_wellPathPoints);
        for (auto& intersection : intersectedCells)
        {
            values[intersection] = RiaDefines::WELL_PATH;
        }
    }

    if (wellPath->fishbonesCollection()->isChecked())
    {
        for (const RimFishbonesMultipleSubs* fishbones : wellPath->fishbonesCollection()->fishbonesSubs)
        {
            if (fishbones->isActive())
            {
                calculateFishbonesIntersections(fishbones, eclipseCaseData, values);
            }
        }
    }

    if (wellPath->fractureCollection()->isChecked())
    {
        const RigMainGrid* grid = eclipseCaseData->mainGrid();

        for (const RimWellPathFracture* fracture : wellPath->fractureCollection()->fractures())
        {
            if (fracture->isChecked())
            {
                calculateFractureIntersections(grid, fracture, values);
            }
        }
    }

    if (wellPath->perforationIntervalCollection()->isChecked())
    {
        for (const RimPerforationInterval* perforationInterval : wellPath->perforationIntervalCollection()->perforations())
        {
            if (perforationInterval->isChecked() && perforationInterval->isActiveOnDate(fromDate))
            {
                calculatePerforationIntersections(wellPath, perforationInterval, eclipseCaseData, values);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCompletionCellIntersectionCalc::calculateFishbonesIntersections(const RimFishbonesMultipleSubs* fishbonesSubs,
                                                                        const RigEclipseCaseData*       eclipseCaseData,
                                                                        std::vector<double>&            values)
{
    for (auto& sub : fishbonesSubs->installedLateralIndices())
    {
        for (size_t lateralIndex : sub.lateralIndices)
        {
            auto intersectedCells = RigWellPathIntersectionTools::findIntersectedGlobalCellIndices(
                eclipseCaseData, fishbonesSubs->coordsForLateral(sub.subIndex, lateralIndex));
            for (auto& intersection : intersectedCells)
            {
                values[intersection] = RiaDefines::FISHBONES;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCompletionCellIntersectionCalc::calculatePerforationIntersections(const RimWellPath*            wellPath,
                                                                          const RimPerforationInterval* perforationInterval,
                                                                          const RigEclipseCaseData*     eclipseCaseData,
                                                                          std::vector<double>&          values)
{
    using namespace std;
    pair<vector<cvf::Vec3d>, vector<double>> clippedWellPathData =
        wellPath->wellPathGeometry()->clippedPointSubset(perforationInterval->startMD(), perforationInterval->endMD());

    auto intersections = RigWellPathIntersectionTools::findIntersectedGlobalCellIndices(
        eclipseCaseData, clippedWellPathData.first, clippedWellPathData.second);
    for (auto& intersection : intersections)
    {
        values[intersection] = RiaDefines::PERFORATION_INTERVAL;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCompletionCellIntersectionCalc::calculateFractureIntersections(const RigMainGrid*   mainGrid,
                                                                       const RimWellPathFracture*   fracture,
                                                                       std::vector<double>& values)
{
    if (!fracture->fractureTemplate()) return;
    if (!fracture->fractureTemplate()->fractureGrid()) return;

    for (const RigFractureCell& fractureCell : fracture->fractureTemplate()->fractureGrid()->fractureCells())
    {
        if (!fractureCell.hasNonZeroConductivity()) continue;

        std::vector<cvf::Vec3d> fractureCellTransformed;
        for (const auto& v : fractureCell.getPolygon())
        {
            cvf::Vec3d polygonNode = v;
            polygonNode.transformPoint(fracture->transformMatrix());
            fractureCellTransformed.push_back(polygonNode);
        }

        std::vector<size_t> potentialCells;

        {
            cvf::BoundingBox boundingBox;

            for (const cvf::Vec3d& nodeCoord : fractureCellTransformed)
            {
                boundingBox.add(nodeCoord);
            }

            mainGrid->findIntersectingCells(boundingBox, &potentialCells);
        }

        for (size_t cellIndex : potentialCells)
        {
            if (!fracture->isEclipseCellWithinContainment(mainGrid, cellIndex)) continue;

            std::array<cvf::Vec3d, 8> hexCorners;
            mainGrid->cellCornerVertices(cellIndex, hexCorners.data());

            std::vector<std::vector<cvf::Vec3d>> planeCellPolygons;

            bool isPlaneIntersected =
                RigHexIntersectionTools::planeHexIntersectionPolygons(hexCorners, fracture->transformMatrix(), planeCellPolygons);
            if (!isPlaneIntersected || planeCellPolygons.empty()) continue;

            {
                cvf::Mat4d invertedTransformMatrix = cvf::Mat4d(fracture->transformMatrix().getInverted());
                for (std::vector<cvf::Vec3d>& planeCellPolygon : planeCellPolygons)
                {
                    for (cvf::Vec3d& v : planeCellPolygon)
                    {
                        v.transformPoint(invertedTransformMatrix);
                    }
                }
            }

            for (const std::vector<cvf::Vec3d>& planeCellPolygon : planeCellPolygons)
            {
                std::vector<std::vector<cvf::Vec3d>> clippedPolygons =
                    RigCellGeometryTools::intersectPolygons(planeCellPolygon, fractureCell.getPolygon());
                for (const auto& clippedPolygon : clippedPolygons)
                {
                    if (!clippedPolygon.empty())
                    {
                        values[cellIndex] = RiaDefines::FRACTURE;
                        break;
                    }
                }
            }
        }
    }
}
