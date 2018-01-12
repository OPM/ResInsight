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
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimSimWellInViewCollection.h"
#include "RimFishbonesCollection.h"
#include "RimFishbonesMultipleSubs.h"
#include "RimOilField.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimProject.h"
#include "RimSimWellInView.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathCompletions.h"

#include "RigMainGrid.h"
#include "RigWellPath.h"
#include "RigWellPathIntersectionTools.h"
#include "RigCellGeometryTools.h"

#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
#include "RimFracture.h"
#include "RimWellPathFracture.h"
#include "RimFractureTemplate.h"
#include "RimWellPathFractureCollection.h"
#include "RimSimWellFractureCollection.h"
#include "RimSimWellFracture.h"
#include "RigFractureGrid.h"
#include "RigFractureCell.h"
#endif // USE_PROTOTYPE_FEATURE_FRACTURES


#include <QDateTime>
#include "RigHexIntersectionTools.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCompletionCellIntersectionCalc::calculateCompletionTypeResult(const RimProject* project, 
                                                                      const RimEclipseCase* eclipseCase, 
                                                                      const RigMainGrid* grid, 
                                                                      std::vector<double>& completionTypeCellResults, 
                                                                      const QDateTime& fromDate)
{
    for (const RimWellPath* wellPath : project->activeOilField()->wellPathCollection->wellPaths)
    {
        if (wellPath->showWellPath())
        {
            calculateWellPathIntersections(wellPath, grid, completionTypeCellResults, fromDate);
        }
    }

#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
    for (RimEclipseView* view : eclipseCase->reservoirViews())
    {
        for (RimSimWellInView* simWell : view->wellCollection()->wells())
        {
            for (RimSimWellFracture* fracture : simWell->simwellFractureCollection()->simwellFractures())
            {
                calculateFractureIntersections(grid, fracture, completionTypeCellResults);
            }
        }
    }
#endif // USE_PROTOTYPE_FEATURE_FRACTURES
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCompletionCellIntersectionCalc::calculateWellPathIntersections(const RimWellPath* wellPath, 
                                                                       const RigMainGrid* grid, 
                                                                       std::vector<double>& values, 
                                                                       const QDateTime& fromDate)
{
    std::vector<HexIntersectionInfo> intersections;
    if (wellPath->wellPathGeometry())
    {
        intersections = RigWellPathIntersectionTools::findRawHexCellIntersections(grid,
                                                                                  wellPath->wellPathGeometry()->m_wellPathPoints);
    }

    for (auto& intersection : intersections)
    {
        values[intersection.m_hexIndex] = RiaDefines::WELL_PATH;
    }

    if (wellPath->fishbonesCollection()->isChecked())
    {
        for (const RimFishbonesMultipleSubs* fishbones : wellPath->fishbonesCollection()->fishbonesSubs)
        {
            if (fishbones->isActive())
            {
                calculateFishbonesIntersections(fishbones, grid, values);
            }
        }
    }

#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
    if (wellPath->fractureCollection()->isChecked())
    {
        for (const RimWellPathFracture* fracture : wellPath->fractureCollection()->fractures())
        {
            if (fracture->isChecked())
            {
                calculateFractureIntersections(grid, fracture, values);
            }
        }
    }
#endif // USE_PROTOTYPE_FEATURE_FRACTURES

    if (wellPath->perforationIntervalCollection()->isChecked())
    {
        for (const RimPerforationInterval* perforationInterval : wellPath->perforationIntervalCollection()->perforations())
        {
            if (perforationInterval->isChecked() && perforationInterval->isActiveOnDate(fromDate))
            {
                calculatePerforationIntersections(wellPath, perforationInterval, grid, values);
            }
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
            std::vector<HexIntersectionInfo> intersections = RigWellPathIntersectionTools::findRawHexCellIntersections(grid, 
                                                                                                                       fishbonesSubs->coordsForLateral(sub.subIndex, lateralIndex));
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
void RimCompletionCellIntersectionCalc::calculatePerforationIntersections(const RimWellPath* wellPath, 
                                                                          const RimPerforationInterval* perforationInterval, 
                                                                          const RigMainGrid* grid, 
                                                                          std::vector<double>& values)
{
    using namespace std;
    pair<vector<cvf::Vec3d>, vector<double> > clippedWellPathData =  wellPath->wellPathGeometry()->clippedPointSubset(perforationInterval->startMD(), 
                                                                                                                      perforationInterval->endMD());

    std::vector<HexIntersectionInfo> intersections = RigWellPathIntersectionTools::findRawHexCellIntersections(grid,
                                                                                                               clippedWellPathData.first);
    for (auto& intersection : intersections)
    {
        values[intersection.m_hexIndex] = RiaDefines::PERFORATION_INTERVAL;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
#ifdef USE_PROTOTYPE_FEATURE_FRACTURES
void RimCompletionCellIntersectionCalc::calculateFractureIntersections(const RigMainGrid* mainGrid, const RimFracture* fracture, std::vector<double>& values)
{
    if (!fracture->fractureTemplate()) return;

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

            for (cvf::Vec3d nodeCoord : fractureCellTransformed)
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

            std::vector< std::vector<cvf::Vec3d> > planeCellPolygons;
            bool isPlaneIntersected = RigHexIntersectionTools::planeHexIntersectionPolygons(hexCorners, fracture->transformMatrix(), planeCellPolygons);
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
                std::vector< std::vector<cvf::Vec3d> > clippedPolygons = RigCellGeometryTools::intersectPolygons(planeCellPolygon, fractureCell.getPolygon());
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
#endif // USE_PROTOTYPE_FEATURE_FRACTURES
