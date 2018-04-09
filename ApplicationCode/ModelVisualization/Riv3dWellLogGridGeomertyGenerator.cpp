/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "Riv3dWellLogGridGeomertyGenerator.h"

#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RigWellPath.h"
#include "RigWellPathGeometryTools.h"

#include "cafDisplayCoordTransform.h"
#include "cvfPrimitiveSetIndexedUInt.h"

#include "cvfBoundingBox.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Riv3dWellLogGridGeometryGenerator::Riv3dWellLogGridGeometryGenerator(RimWellPath* wellPath)
    : m_wellPath(wellPath)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> Riv3dWellLogGridGeometryGenerator::createGrid(const caf::DisplayCoordTransform* displayCoordTransform,
                                                                          const cvf::BoundingBox& wellPathClipBoundingBox,
                                                                          double                  planeAngle,
                                                                          double                  planeOffsetFromWellPathCenter,
                                                                          double                  planeWidth,
                                                                          double                  gridIntervalSize) const
{
    CVF_ASSERT(gridIntervalSize > 0);

    if (!wellPathGeometry()) return nullptr;
    if (!wellPathClipBoundingBox.isValid()) return nullptr;

    RimWellPathCollection* wellPathCollection = nullptr;
    m_wellPath->firstAncestorOrThisOfTypeAsserted(wellPathCollection);

    std::vector<cvf::Vec3d> wellPathPoints = wellPathGeometry()->m_wellPathPoints;
    if (wellPathPoints.empty()) return nullptr;

    size_t originalWellPathSize = wellPathPoints.size();

    if (wellPathCollection->wellPathClip)
    {
        double horizontalLengthAlongWellToClipPoint;
        double maxZClipHeight = wellPathClipBoundingBox.max().z() + wellPathCollection->wellPathClipZDistance;
        size_t indexToFirstVisibleSegment;
        wellPathPoints = RigWellPath::clipPolylineStartAboveZ(
            wellPathPoints, maxZClipHeight, &horizontalLengthAlongWellToClipPoint, &indexToFirstVisibleSegment);
    }
    if (wellPathPoints.empty()) return nullptr;

    std::vector<cvf::Vec3d> gridPoints;

    if (wellPathGeometry()->m_measuredDepths.empty()) return nullptr;

    size_t newStartIndex = originalWellPathSize - wellPathPoints.size();
    double firstMd = wellPathGeometry()->m_measuredDepths.at(newStartIndex);
    double lastMd = wellPathGeometry()->m_measuredDepths.back();

    double md = lastMd;
    while (md >= firstMd)
    {
        cvf::Vec3d point = wellPathGeometry()->interpolatedPointAlongWellPath(md);
        gridPoints.push_back(point);
        md -= gridIntervalSize;
    }

    std::vector<cvf::Vec3d> pointNormals;

    std::vector<cvf::Vec3d> closestPoints;
    RigWellPathGeometryTools::calculatePairsOfClosestSamplingPointsAlongWellPath(wellPathGeometry(), gridPoints, &closestPoints);

    pointNormals = RigWellPathGeometryTools::calculateLineSegmentNormals(wellPathGeometry(), planeAngle, closestPoints, RigWellPathGeometryTools::LINE_SEGMENTS);
    if (pointNormals.size() != gridPoints.size()) return nullptr;

    std::vector<cvf::Vec3f> vertices;
    vertices.reserve(gridPoints.size() * 2);

    std::vector<cvf::uint> indices;
    indices.reserve(gridPoints.size() * 2);

    cvf::uint indexCounter = 0;

    // Normal lines
    for (size_t i = 0; i < pointNormals.size(); i++)
    {
        vertices.push_back(cvf::Vec3f(
            displayCoordTransform->transformToDisplayCoord(gridPoints[i] + pointNormals[i] * planeOffsetFromWellPathCenter)));

        vertices.push_back(cvf::Vec3f(displayCoordTransform->transformToDisplayCoord(
            gridPoints[i] + pointNormals[i] * (planeOffsetFromWellPathCenter + planeWidth))));

        indices.push_back(indexCounter++);
        indices.push_back(indexCounter++);
    }

    // calculateLineSegmentNormals returns normals for the whole well path. Erase the part which is clipped off
    std::vector<cvf::Vec3d> wellPathSegmentNormals =
        RigWellPathGeometryTools::calculateLineSegmentNormals(wellPathGeometry(), planeAngle, wellPathGeometry()->m_wellPathPoints, RigWellPathGeometryTools::POLYLINE);
    wellPathSegmentNormals.erase(wellPathSegmentNormals.begin(), wellPathSegmentNormals.end() - wellPathPoints.size());

    // Line along and close to well
    for (size_t i = 0; i < wellPathPoints.size(); i++)
    {
        vertices.push_back(cvf::Vec3f(displayCoordTransform->transformToDisplayCoord(
            wellPathPoints[i] + wellPathSegmentNormals[i] * planeOffsetFromWellPathCenter)));

        indices.push_back(indexCounter);
        indices.push_back(++indexCounter);
    }
    // Indices are added as line segments for the current point and the next point. The last point does not have a next point,
    // therefore we remove the last line segment
    indices.pop_back();
    indices.pop_back();

    // Line along and far away from well
    for (size_t i = 0; i < wellPathPoints.size(); i++)
    {
        vertices.push_back(cvf::Vec3f(displayCoordTransform->transformToDisplayCoord(
            wellPathPoints[i] + wellPathSegmentNormals[i] * (planeOffsetFromWellPathCenter + planeWidth))));

        indices.push_back(indexCounter);
        indices.push_back(++indexCounter);
    }
    indices.pop_back();
    indices.pop_back();

    cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUInt = new cvf::PrimitiveSetIndexedUInt(cvf::PrimitiveType::PT_LINES);
    cvf::ref<cvf::UIntArray>               indexArray = new cvf::UIntArray(indices);

    cvf::ref<cvf::DrawableGeo> drawable = new cvf::DrawableGeo();

    indexedUInt->setIndices(indexArray.p());
    drawable->addPrimitiveSet(indexedUInt.p());

    cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray(vertices);
    drawable->setVertexArray(vertexArray.p());

    return drawable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigWellPath* Riv3dWellLogGridGeometryGenerator::wellPathGeometry() const
{
    return m_wellPath->wellPathGeometry();
}
