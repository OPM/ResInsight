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
#include "cvfMath.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Riv3dWellLogCurveGeometryGenerator::Riv3dWellLogCurveGeometryGenerator(RimWellPath* wellPath)
    : m_wellPath(wellPath)
    , m_planeWidth(0.0)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Riv3dWellLogCurveGeometryGenerator::createCurveDrawables(const caf::DisplayCoordTransform* displayCoordTransform,
                                                              const cvf::BoundingBox&           wellPathClipBoundingBox,
                                                              const std::vector<double>&        resultValues,
                                                              const std::vector<double>&        resultMds,
                                                              double                            minResultValue,
                                                              double                            maxResultValue,
                                                              double                            planeAngle,
                                                              double                            planeOffsetFromWellPathCenter,
                                                              double                            planeWidth)
{
    m_planeWidth = planeWidth;

    if (!wellPathGeometry()) return;
    if (wellPathGeometry()->m_wellPathPoints.empty()) return;
    if (!wellPathClipBoundingBox.isValid()) return;

    if (resultValues.empty()) return;
    CVF_ASSERT(resultValues.size() == resultMds.size());

    if (maxResultValue - minResultValue < 1.0e-6)
    {
        return;
    }

    RimWellPathCollection* wellPathCollection = nullptr;
    m_wellPath->firstAncestorOrThisOfTypeAsserted(wellPathCollection);

    cvf::Vec3d clipLocation = wellPathGeometry()->m_wellPathPoints.front();
    if (wellPathCollection->wellPathClip)
    {
        double clipZDistance = wellPathCollection->wellPathClipZDistance;
        clipLocation = wellPathClipBoundingBox.max() + clipZDistance * cvf::Vec3d(0, 0, 1);
    }
    clipLocation = displayCoordTransform->transformToDisplayCoord(clipLocation);

    std::vector<cvf::Vec3d> wellPathPoints = wellPathGeometry()->m_wellPathPoints;
    for (cvf::Vec3d& wellPathPoint : wellPathPoints)
    {
        wellPathPoint = displayCoordTransform->transformToDisplayCoord(wellPathPoint);
    }

    std::vector<cvf::Vec3d> wellPathCurveNormals = RigWellPathGeometryTools::calculateLineSegmentNormals(wellPathPoints, planeAngle);

    std::vector<cvf::Vec3d> interpolatedWellPathPoints;
    std::vector<cvf::Vec3d> interpolatedCurveNormals;
    // Iterate from bottom of well path and up to be able to stop at given Z max clipping height
    for (auto md = resultMds.rbegin(); md != resultMds.rend(); md++)
    {
        cvf::Vec3d point = wellPathGeometry()->interpolatedVectorAlongWellPath(wellPathPoints, *md);
        cvf::Vec3d normal = wellPathGeometry()->interpolatedVectorAlongWellPath(wellPathCurveNormals, *md);
        if (point.z() > clipLocation.z()) break;

        interpolatedWellPathPoints.push_back(point);
        interpolatedCurveNormals.push_back(normal.getNormalized());
    }
    if (interpolatedWellPathPoints.empty()) return;

    // Reverse list, since it was filled in the opposite order
    std::reverse(interpolatedWellPathPoints.begin(), interpolatedWellPathPoints.end());
    std::reverse(interpolatedCurveNormals.begin(), interpolatedCurveNormals.end());

    // The result values for the part of the well which is not clipped off, matching interpolatedWellPathPoints size
    m_curveValues = std::vector<double>(resultValues.end() - interpolatedWellPathPoints.size(),
                                        resultValues.end());
    m_curveMeasuredDepths = std::vector<double>(resultMds.end() - interpolatedWellPathPoints.size(),
                                                resultMds.end());

    double maxClampedResult = -HUGE_VAL;
    double minClampedResult = HUGE_VAL;

    for (double& result : m_curveValues)
    {
        if (!RigCurveDataTools::isValidValue(result, false)) continue;

        result = cvf::Math::clamp(result, minResultValue, maxResultValue);

        maxClampedResult = std::max(result, maxClampedResult);
        minClampedResult = std::min(result, minClampedResult);
    }

    if (minClampedResult >= maxClampedResult)
    {
        return;
    }

    m_curveVertices = std::vector<cvf::Vec3f>();
    m_curveVertices.reserve(interpolatedWellPathPoints.size());

    double plotRangeToResultRangeFactor = planeWidth / (maxClampedResult - minClampedResult);

    for (size_t i = 0; i < interpolatedWellPathPoints.size(); i++)
    {
        double scaledResult = 0;

        if (RigCurveDataTools::isValidValue(m_curveValues[i], false))
        {
            scaledResult =
                planeOffsetFromWellPathCenter + (m_curveValues[i] - minClampedResult) * plotRangeToResultRangeFactor;
        }
        cvf::Vec3d curvePoint(interpolatedWellPathPoints[i] + scaledResult * interpolatedCurveNormals[i]);
        m_curveVertices.push_back(cvf::Vec3f(curvePoint));
    }

    std::vector<cvf::uint> indices;
    indices.reserve(interpolatedWellPathPoints.size());
    for (size_t i = 0; i < m_curveValues.size() - 1; ++i)
    {
        if (RigCurveDataTools::isValidValue(m_curveValues[i], false) &&
            RigCurveDataTools::isValidValue(m_curveValues[i + 1], false))
        {
            indices.push_back(cvf::uint(i));
            indices.push_back(cvf::uint(i + 1));
        }
    }
    
    cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUInt = new cvf::PrimitiveSetIndexedUInt(cvf::PrimitiveType::PT_LINES);
    cvf::ref<cvf::UIntArray>               indexArray = new cvf::UIntArray(indices);

    m_curveDrawable = new cvf::DrawableGeo();

    indexedUInt->setIndices(indexArray.p());
    m_curveDrawable->addPrimitiveSet(indexedUInt.p());

    cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray(m_curveVertices);
    m_curveDrawable->setVertexArray(vertexArray.p());    
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> Riv3dWellLogCurveGeometryGenerator::curveDrawable()
{
    return m_curveDrawable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigWellPath* Riv3dWellLogCurveGeometryGenerator::wellPathGeometry() const
{
    return m_wellPath->wellPathGeometry();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool Riv3dWellLogCurveGeometryGenerator::findClosestPointOnCurve(const cvf::Vec3d& globalIntersection,
                                                                 cvf::Vec3d*       closestPoint,
                                                                 double*           measuredDepthAtPoint,
                                                                 double*           valueAtClosestPoint) const
{
    cvf::Vec3f globalIntersectionFloat(globalIntersection);
    float closestDistance = m_planeWidth * 0.1;
    *closestPoint = cvf::Vec3d::UNDEFINED;
    *measuredDepthAtPoint = cvf::UNDEFINED_DOUBLE;
    *valueAtClosestPoint = cvf::UNDEFINED_DOUBLE;
    if (m_curveVertices.size() < 2u) false;
    CVF_ASSERT(m_curveVertices.size() == m_curveValues.size());
    for (size_t i = 1; i < m_curveVertices.size(); ++i)
    {
        bool validCurveSegment = RigCurveDataTools::isValidValue(m_curveValues[i], false) &&
                                 RigCurveDataTools::isValidValue(m_curveValues[i - 1], false);
        if (validCurveSegment)
        {
            cvf::Vec3f a = m_curveVertices[i - 1];
            cvf::Vec3f b = m_curveVertices[i];
            cvf::Vec3f ap = globalIntersectionFloat - a;
            cvf::Vec3f ab = b - a;
            // Projected point is clamped to one of the end points of the segment.
            float distanceToProjectedPointAlongAB = ap * ab / (ab * ab);
            float clampedDistance = cvf::Math::clamp(distanceToProjectedPointAlongAB, 0.0f, 1.0f);
            cvf::Vec3f projectionOfGlobalIntersection = a + clampedDistance * ab;
            float distance = (projectionOfGlobalIntersection - globalIntersectionFloat).length();
            if (distance < closestDistance)
            {
                *closestPoint = cvf::Vec3d(projectionOfGlobalIntersection);
                closestDistance = distance;
                *measuredDepthAtPoint = m_curveMeasuredDepths[i - 1] * (1.0f - clampedDistance) + m_curveMeasuredDepths[i] * clampedDistance;
                *valueAtClosestPoint = m_curveValues[i - 1] * (1.0f - clampedDistance) + m_curveValues[i] * clampedDistance;
            }
        }
    }

    if (closestPoint->isUndefined())
        return false;
    
    return true;
}
