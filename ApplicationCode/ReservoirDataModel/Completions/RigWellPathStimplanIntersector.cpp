/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RigWellPathStimplanIntersector.h"

#include "RigCellGeometryTools.h"
#include "RigFractureCell.h"
#include "RigFractureGrid.h"
#include "RigWellPath.h"

#include "RimFracture.h"
#include "RimFractureTemplate.h"
#include "RimSimWellFracture.h"
#include "RimStimPlanFractureTemplate.h"

#include "cvfBase.h"
#include "cvfMath.h"
#include "cvfMatrix4.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellPathStimplanIntersector::RigWellPathStimplanIntersector(const RigWellPath* wellPathGeom, const RimFracture* rimFracture)
{
    std::vector<cvf::Vec3d> wellPathPoints =
        wellPathGeom->wellPathPointsIncludingInterpolatedIntersectionPoint(rimFracture->fractureMD());
    cvf::Mat4d fractureXf = rimFracture->transformMatrix();
    double     wellRadius = rimFracture->wellRadius();

    std::vector<std::vector<cvf::Vec3d>> fractureGridCellPolygons;
    {
        RimFractureTemplate* fractureTemplate = rimFracture->fractureTemplate();

        if (fractureTemplate && fractureTemplate->fractureGrid())
        {
            const std::vector<RigFractureCell>& stpCells = fractureTemplate->fractureGrid()->fractureCells();
            for (const auto& stpCell : stpCells)
            {
                fractureGridCellPolygons.push_back(stpCell.getPolygon());
            }
        }
    }
    double perforationLength = rimFracture->perforationLength();

    calculate(fractureXf,
              wellPathPoints,
              wellRadius,
              perforationLength,
              fractureGridCellPolygons,
              m_stimPlanCellIdxToIntersectionInfoMap);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::map<size_t, RigWellPathStimplanIntersector::RigWellPathStimplanIntersector::WellCellIntersection>&
    RigWellPathStimplanIntersector::intersections() const
{
    return m_stimPlanCellIdxToIntersectionInfoMap;
}

//--------------------------------------------------------------------------------------------------
///  Todo: Use only the perforated parts of the well path
//--------------------------------------------------------------------------------------------------
void RigWellPathStimplanIntersector::calculate(const cvf::Mat4d&                           fractureXf,
                                               const std::vector<cvf::Vec3d>&              wellPathPointsDomainCoords,
                                               double                                      wellRadius,
                                               double                                      perforationLength,
                                               const std::vector<std::vector<cvf::Vec3d>>& fractureGridCellPolygons,
                                               std::map<size_t, WellCellIntersection>&     m_stimPlanCellIdxToIntersectionInfoMap)
{
    cvf::Mat4d toFractureXf = fractureXf.getInverted();

    std::vector<cvf::Vec3d> perforationLengthBoundingBoxPolygon;
    {
        double cicleRadius           = perforationLength / 2;
        int    pointsInCirclePolygon = 20;

        for (int i = 0; i < pointsInCirclePolygon; i++)
        {
            double x = cicleRadius * cvf::Math::cos(i * (2 * cvf::PI_D / pointsInCirclePolygon));
            double y = cicleRadius * cvf::Math::sin(i * (2 * cvf::PI_D / pointsInCirclePolygon));
            perforationLengthBoundingBoxPolygon.push_back(cvf::Vec3d(x, y, 0));
        }
    }

    // Convert well path to fracture template system

    std::vector<cvf::Vec3d> fractureRelativeWellPathPoints;
    for (const auto& wellPPoint : wellPathPointsDomainCoords)
    {
        fractureRelativeWellPathPoints.push_back(wellPPoint.getTransformedPoint(toFractureXf));
    }

    // Clip well path to fracture domain

    std::vector<std::vector<cvf::Vec3d>> wellPathPartsWithinFracture = RigCellGeometryTools::clipPolylineByPolygon(
        fractureRelativeWellPathPoints, perforationLengthBoundingBoxPolygon, RigCellGeometryTools::INTERPOLATE_LINE_Z);

    // Remove the part of the well path that is more than well radius away from the fracture plane

    std::vector<std::vector<cvf::Vec3d>> intersectingWellPathParts;

    for (const auto& part : wellPathPartsWithinFracture)
    {
        std::vector<cvf::Vec3d> currentIntersectingWpPart;
        for (size_t vxIdx = 0; vxIdx < part.size() - 1; ++vxIdx)
        {
            double thisAbsZ = fabs(part[vxIdx].z());
            double nextAbsZ = fabs(part[vxIdx + 1].z());
            double thisZ    = part[vxIdx].z();
            double nextZ    = part[vxIdx + 1].z();

            if (thisAbsZ >= wellRadius && nextAbsZ >= wellRadius)
            {
                if ((thisZ >= 0 && nextZ >= 0) || (thisZ <= 0 && nextZ <= 0))
                {
                    continue; // Outside
                }
                else // In and out
                {
                    {
                        double wellRadiusDistFromPlane = thisZ > 0 ? wellRadius : -wellRadius;

                        double fraction = (wellRadiusDistFromPlane - thisZ) / (nextZ - thisZ);

                        cvf::Vec3d intersectPoint = part[vxIdx] + fraction * (part[vxIdx + 1] - part[vxIdx]);
                        currentIntersectingWpPart.push_back(intersectPoint);
                    }
                    {
                        double wellRadiusDistFromPlane = nextZ > 0 ? wellRadius : -wellRadius;

                        double fraction = (wellRadiusDistFromPlane - thisZ) / (nextZ - thisZ);

                        cvf::Vec3d intersectPoint = part[vxIdx] + fraction * (part[vxIdx + 1] - part[vxIdx]);
                        currentIntersectingWpPart.push_back(intersectPoint);

                        intersectingWellPathParts.push_back(currentIntersectingWpPart);
                        currentIntersectingWpPart.clear();
                    }
                    continue;
                }
            }
            if (thisAbsZ < wellRadius && nextAbsZ < wellRadius) // Inside
            {
                currentIntersectingWpPart.push_back(part[vxIdx]);
                continue;
            }

            if (thisAbsZ < wellRadius && nextAbsZ >= wellRadius) // Going out
            {
                currentIntersectingWpPart.push_back(part[vxIdx]);

                double wellRadiusDistFromPlane = nextZ > 0 ? wellRadius : -wellRadius;

                double fraction = (wellRadiusDistFromPlane - thisZ) / (nextZ - thisZ);

                cvf::Vec3d intersectPoint = part[vxIdx] + fraction * (part[vxIdx + 1] - part[vxIdx]);
                currentIntersectingWpPart.push_back(intersectPoint);

                intersectingWellPathParts.push_back(currentIntersectingWpPart);
                currentIntersectingWpPart.clear();
                continue;
            }

            if (thisAbsZ >= wellRadius && nextAbsZ < wellRadius) // Going in
            {
                double wellRadiusDistFromPlane = thisZ > 0 ? wellRadius : -wellRadius;

                double fraction = (wellRadiusDistFromPlane - thisZ) / (nextZ - thisZ);

                cvf::Vec3d intersectPoint = part[vxIdx] + fraction * (part[vxIdx + 1] - part[vxIdx]);
                currentIntersectingWpPart.push_back(intersectPoint);
                continue;
            }
        }

        // Add last point if it is within the radius

        if (part.size() > 1 && fabs(part.back().z()) < wellRadius)
        {
            currentIntersectingWpPart.push_back(part.back());
        }

        if (!currentIntersectingWpPart.empty())
        {
            intersectingWellPathParts.push_back(currentIntersectingWpPart);
        }
    }

    // Find the StimPlan cells touched by the intersecting well path parts

    for (size_t cIdx = 0; cIdx < fractureGridCellPolygons.size(); ++cIdx)
    {
        const std::vector<cvf::Vec3d>& cellPolygon = fractureGridCellPolygons[cIdx];
        for (const auto& wellpathPart : intersectingWellPathParts)
        {
            std::vector<std::vector<cvf::Vec3d>> wellPathPartsInPolygon =
                RigCellGeometryTools::clipPolylineByPolygon(wellpathPart, cellPolygon, RigCellGeometryTools::USE_HUGEVAL);
            for (const auto& wellPathPartInCell : wellPathPartsInPolygon)
            {
                if (!wellPathPartInCell.empty())
                {
                    int endpointCount = 0;
                    if (wellPathPartInCell.front().z() != HUGE_VAL) ++endpointCount;
                    if (wellPathPartInCell.back().z() != HUGE_VAL) ++endpointCount;

                    cvf::Vec3d intersectionLength = (wellPathPartInCell.back() - wellPathPartInCell.front());
                    double     xLengthInCell      = fabs(intersectionLength.x());
                    double     yLengthInCell      = fabs(intersectionLength.y());

                    m_stimPlanCellIdxToIntersectionInfoMap[cIdx].endpointCount += endpointCount;
                    m_stimPlanCellIdxToIntersectionInfoMap[cIdx].hlength += xLengthInCell;
                    m_stimPlanCellIdxToIntersectionInfoMap[cIdx].vlength += yLengthInCell;
                }
            }
        }
    }
}
