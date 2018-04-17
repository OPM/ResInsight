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

#include "RigWellPathIntersectionTools.h"

#include "RiaLogging.h"

#include "RigCellGeometryTools.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigMainGrid.h"
#include "RigSimulationWellCoordsAndMD.h"
#include "RigWellLogExtractionTools.h"
#include "RigWellPath.h"

#include "RimEclipseCase.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<WellPathCellIntersectionInfo> RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath(const RigEclipseCaseData* caseData,
                                                                                                           const std::vector<cvf::Vec3d>& pathCoords,
                                                                                                           const std::vector<double>& pathMds)
{
    std::vector<WellPathCellIntersectionInfo> intersectionInfos;
    const RigMainGrid* grid = caseData->mainGrid();

    if (pathCoords.size() < 2) return intersectionInfos;

    cvf::ref<RigWellPath> dummyWellPath = new RigWellPath;
    dummyWellPath->m_wellPathPoints = pathCoords;
    dummyWellPath->m_measuredDepths = pathMds;

    cvf::ref<RigEclipseWellLogExtractor> extractor = new RigEclipseWellLogExtractor(caseData, 
                                                                                    dummyWellPath.p(), 
                                                                                    caseData->ownerCase()->caseUserDescription().toStdString());

    return extractor->cellIntersectionInfosAlongWellPath();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<size_t> RigWellPathIntersectionTools::findIntersectedGlobalCellIndicesForWellPath(const RigEclipseCaseData* caseData, const RigWellPath* wellPath)
{
    std::set<size_t> globalCellIndices;

    if (caseData)
    {
        cvf::ref<RigEclipseWellLogExtractor> extractor = new RigEclipseWellLogExtractor(caseData,
                                                                                        wellPath,
                                                                                        caseData->ownerCase()->caseUserDescription().toStdString());

        std::vector<WellPathCellIntersectionInfo> intersections = extractor->cellIntersectionInfosAlongWellPath();
        for (const auto& intersection : intersections)
        {
            globalCellIndices.insert(intersection.globCellIndex);
        }
    }

    return globalCellIndices;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<size_t> RigWellPathIntersectionTools::findIntersectedGlobalCellIndices(const RigEclipseCaseData*      caseData,
                                                                                const std::vector<cvf::Vec3d>& coords,
                                                                                const std::vector<double>&     measuredDepths)
{
    std::set<size_t> globalCellIndices;

    if (caseData)
    {
        cvf::ref<RigWellPath> dummyWellPath = new RigWellPath;

        if (measuredDepths.size() == coords.size())
        {
            dummyWellPath->m_wellPathPoints = coords;
            dummyWellPath->m_measuredDepths = measuredDepths;
        }
        else
        {
            RigSimulationWellCoordsAndMD helper(coords);

            dummyWellPath->m_wellPathPoints = helper.wellPathPoints();
            dummyWellPath->m_measuredDepths = helper.measuredDepths();
        }

        globalCellIndices = findIntersectedGlobalCellIndicesForWellPath(caseData, dummyWellPath.p());
    }

    return globalCellIndices;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigWellPathIntersectionTools::calculateLengthInCell(const std::array<cvf::Vec3d, 8>& hexCorners, 
                                                               const cvf::Vec3d& startPoint, 
                                                               const cvf::Vec3d& endPoint)
{
    cvf::Vec3d vec = endPoint - startPoint;
    cvf::Vec3d iAxisDirection;
    cvf::Vec3d jAxisDirection;
    cvf::Vec3d kAxisDirection;

    RigCellGeometryTools::findCellLocalXYZ(hexCorners, iAxisDirection, jAxisDirection, kAxisDirection);

    cvf::Mat3d localCellCoordinateSystem(iAxisDirection.x(), jAxisDirection.x(), kAxisDirection.x(),
                                         iAxisDirection.y(), jAxisDirection.y(), kAxisDirection.y(),
                                         iAxisDirection.z(), jAxisDirection.z(), kAxisDirection.z());

    return vec.getTransformedVector(localCellCoordinateSystem.getInverted());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RigWellPathIntersectionTools::calculateLengthInCell(const RigMainGrid* grid, 
                                                               size_t cellIndex, 
                                                               const cvf::Vec3d& startPoint, 
                                                               const cvf::Vec3d& endPoint)
{
    std::array<cvf::Vec3d, 8> hexCorners;
    grid->cellCornerVertices(cellIndex, hexCorners.data());

    return calculateLengthInCell(hexCorners, startPoint, endPoint);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RigWellPathIntersectionTools::findCloseCells(const RigMainGrid* grid, const cvf::BoundingBox& bb)
{
    std::vector<size_t> closeCells;
    grid->findIntersectingCells(bb, &closeCells);
    return closeCells;
}

