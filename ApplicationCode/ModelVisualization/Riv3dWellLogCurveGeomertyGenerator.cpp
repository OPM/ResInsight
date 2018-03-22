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

#include "Riv3dWellLogCurveGeomertyGenerator.h"

#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RigCurveDataTools.h"
#include "RigWellPath.h"
#include "RigWellPathGeometryTools.h"

#include "cafDisplayCoordTransform.h"
#include "cvfPrimitiveSetIndexedUInt.h"

#include "cvfBoundingBox.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Riv3dWellLogCurveGeometryGenerator::Riv3dWellLogCurveGeometryGenerator(RimWellPath* wellPath)
    : m_wellPath(wellPath)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo>
    Riv3dWellLogCurveGeometryGenerator::createCurveLine(const caf::DisplayCoordTransform* displayCoordTransform,
                                                        const cvf::BoundingBox&           wellPathClipBoundingBox,
                                                        const std::vector<double>&        resultValues,
                                                        const std::vector<double>&        resultMds,
                                                        double                            planeAngle,
                                                        double                            planeOffsetFromWellPathCenter,
                                                        double                            planeWidth) const
{
    std::vector<cvf::Vec3f> vertices;
    std::vector<cvf::uint>  indices;

    createCurveVerticesAndIndices(resultValues,
                                  resultMds,
                                  planeAngle,
                                  planeOffsetFromWellPathCenter,
                                  planeWidth,
                                  displayCoordTransform,
                                  wellPathClipBoundingBox,
                                  &vertices,
                                  &indices);

    if (vertices.empty() || indices.empty())
    {
        return nullptr;
    }

    cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUInt = new cvf::PrimitiveSetIndexedUInt(cvf::PrimitiveType::PT_LINES);
    cvf::ref<cvf::UIntArray>               indexArray  = new cvf::UIntArray(indices);

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
void Riv3dWellLogCurveGeometryGenerator::createCurveVerticesAndIndices(const std::vector<double>&        resultValues,
                                                                       const std::vector<double>&        resultMds,
                                                                       double                            planeAngle,
                                                                       double                            planeOffsetFromWellPathCenter,
                                                                       double                            planeWidth,
                                                                       const caf::DisplayCoordTransform* displayCoordTransform,
                                                                       const cvf::BoundingBox&           wellPathClipBoundingBox,
                                                                       std::vector<cvf::Vec3f>*          vertices,
                                                                       std::vector<cvf::uint>*           indices) const
{
    if (!wellPathGeometry()) return;
    if (wellPathGeometry()->m_wellPathPoints.empty()) return;
    if (!wellPathClipBoundingBox.isValid()) return;

    if (resultValues.empty()) return;
    CVF_ASSERT(resultValues.size() == resultMds.size());

    RimWellPathCollection* wellPathCollection = nullptr;
    m_wellPath->firstAncestorOrThisOfTypeAsserted(wellPathCollection);

    double maxZClipHeight = wellPathGeometry()->m_wellPathPoints.front().z();
    if (wellPathCollection->wellPathClip)
    {
        maxZClipHeight = wellPathClipBoundingBox.max().z() + wellPathCollection->wellPathClipZDistance;
    }

    std::vector<cvf::Vec3d> interpolatedWellPathPoints;

    // Iterate from bottom of well path and up to be able to stop at given Z max clipping height
    for (auto md = resultMds.rbegin(); md != resultMds.rend(); md++)
    {
        cvf::Vec3d point = wellPathGeometry()->interpolatedPointAlongWellPath(*md);
        if (point.z() > maxZClipHeight) break;

        interpolatedWellPathPoints.push_back(point);
    }
    if (interpolatedWellPathPoints.empty()) return;

    // Reverse list, since it was filled in the opposite order
    std::reverse(interpolatedWellPathPoints.begin(), interpolatedWellPathPoints.end());

    // The result values for the part of the well which is not clipped off, matching interpolatedWellPathPoints size
    std::vector<double> resultValuesForInterpolatedPoints(resultValues.end() - interpolatedWellPathPoints.size(),
                                                          resultValues.end());

    std::vector<cvf::Vec3d> pairsOfWellPathPoints;
    RigWellPathGeometryTools::calculatePairsOfClosestSamplingPointsAlongWellPath(wellPathGeometry(), &pairsOfWellPathPoints, interpolatedWellPathPoints);

    std::vector<cvf::Vec3d> pointNormals = RigWellPathGeometryTools::calculateLineSegmentNormals(wellPathGeometry(), planeAngle, pairsOfWellPathPoints, RigWellPathGeometryTools::LINE_SEGMENTS);
    if (interpolatedWellPathPoints.size() != pointNormals.size()) return;

    double maxResult = -HUGE_VAL;
    double minResult = HUGE_VAL;

    for (double result : resultValuesForInterpolatedPoints)
    {
        if (!RigCurveDataTools::isValidValue(result, false)) continue;

        maxResult = std::max(result, maxResult);
        minResult = std::min(result, minResult);
    }

    vertices->resize(interpolatedWellPathPoints.size());

    double plotRangeToResultRangeFactor = planeWidth / (maxResult - minResult);

    for (size_t i = 0; i < pointNormals.size(); i++)
    {
        double scaledResult = 0;

        if (RigCurveDataTools::isValidValue(resultValuesForInterpolatedPoints[i], false))
        {
            scaledResult =
                planeOffsetFromWellPathCenter + (resultValuesForInterpolatedPoints[i] - minResult) * plotRangeToResultRangeFactor;
        }

        (*vertices)[i] = cvf::Vec3f(
            displayCoordTransform->transformToDisplayCoord(interpolatedWellPathPoints[i] + scaledResult * pointNormals[i]));
    }

    std::vector<std::pair<size_t, size_t>> valuesIntervals =
        RigCurveDataTools::calculateIntervalsOfValidValues(resultValuesForInterpolatedPoints, false);

    for (const std::pair<size_t, size_t>& interval : valuesIntervals)
    {
        for (size_t i = interval.first; i < interval.second; i++)
        {
            indices->push_back(cvf::uint(i));
            indices->push_back(cvf::uint(i + 1));
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigWellPath* Riv3dWellLogCurveGeometryGenerator::wellPathGeometry() const
{
    return m_wellPath->wellPathGeometry();
}
