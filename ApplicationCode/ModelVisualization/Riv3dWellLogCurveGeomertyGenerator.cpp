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

#include "cafLine.h"
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
                                                              const Rim3dWellLogCurve*          rim3dWellLogCurve,
                                                              double                            planeOffsetFromWellPathCenter,
                                                              double                            planeWidth,
                                                              const std::vector<cvf::Vec3f>&    gridVertices)
{
    // Make sure all drawables are cleared in case we return early to avoid a
    // previous drawable being "stuck" when changing result type.
    clearCurvePointsAndGeometry();

    std::vector<double> resultValues;
    std::vector<double> resultMds;
    rim3dWellLogCurve->curveValuesAndMds(&resultValues, &resultMds);

    m_planeWidth = planeWidth;

    if (!wellPathGeometry()) return;
    if (wellPathGeometry()->m_wellPathPoints.empty()) return;
    if (!wellPathClipBoundingBox.isValid()) return;

    if (resultValues.empty()) return;
    CVF_ASSERT(resultValues.size() == resultMds.size());

    if (rim3dWellLogCurve->maxCurveValue() - rim3dWellLogCurve->minCurveValue() < 1.0e-6)
    {
        return;
    }

    RimWellPathCollection* wellPathCollection = nullptr;
    m_wellPath->firstAncestorOrThisOfTypeAsserted(wellPathCollection);

    cvf::Vec3d clipLocation = wellPathGeometry()->m_wellPathPoints.front();
    if (wellPathCollection->wellPathClip)
    {
        double clipZDistance = wellPathCollection->wellPathClipZDistance;
        clipLocation         = wellPathClipBoundingBox.max() + clipZDistance * cvf::Vec3d(0, 0, 1);
    }
    clipLocation = displayCoordTransform->transformToDisplayCoord(clipLocation);

    std::vector<cvf::Vec3d> wellPathPoints = wellPathGeometry()->m_wellPathPoints;
    for (cvf::Vec3d& wellPathPoint : wellPathPoints)
    {
        wellPathPoint = displayCoordTransform->transformToDisplayCoord(wellPathPoint);
    }

    std::vector<cvf::Vec3d> wellPathCurveNormals =
        RigWellPathGeometryTools::calculateLineSegmentNormals(wellPathPoints, rim3dWellLogCurve->drawPlaneAngle());

    std::vector<cvf::Vec3d> interpolatedWellPathPoints;
    std::vector<cvf::Vec3d> interpolatedCurveNormals;
    // Iterate from bottom of well path and up to be able to stop at given Z max clipping height
    for (auto md = resultMds.rbegin(); md != resultMds.rend(); md++)
    {
        cvf::Vec3d point  = wellPathGeometry()->interpolatedVectorAlongWellPath(wellPathPoints, *md);
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
    m_curveValues         = std::vector<double>(resultValues.end() - interpolatedWellPathPoints.size(), resultValues.end());
    m_curveMeasuredDepths = std::vector<double>(resultMds.end() - interpolatedWellPathPoints.size(), resultMds.end());

    double maxClampedResult = -HUGE_VAL;
    double minClampedResult = HUGE_VAL;

    for (double& result : m_curveValues)
    {
        if (!RigCurveDataTools::isValidValue(result, false)) continue;

        result = cvf::Math::clamp(result, rim3dWellLogCurve->minCurveValue(), rim3dWellLogCurve->maxCurveValue());

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
            scaledResult = planeOffsetFromWellPathCenter + (m_curveValues[i] - minClampedResult) * plotRangeToResultRangeFactor;
        }
        cvf::Vec3d curvePoint(interpolatedWellPathPoints[i] + scaledResult * interpolatedCurveNormals[i]);
        m_curveVertices.push_back(cvf::Vec3f(curvePoint));
    }

    createNewVerticesAlongTriangleEdges(gridVertices);
    projectVerticesOntoTriangles(gridVertices);
   

    std::vector<cvf::uint> indices;
    indices.reserve(m_curveVertices.size() * 2);
    for (size_t i = 0; i < m_curveVertices.size() - 1; ++i)
    {
        indices.push_back(cvf::uint(i));
        indices.push_back(cvf::uint(i + 1));
    }

    cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUInt = new cvf::PrimitiveSetIndexedUInt(cvf::PrimitiveType::PT_LINES);
    cvf::ref<cvf::UIntArray>               indexArray  = new cvf::UIntArray(indices);

    m_curveDrawable = new cvf::DrawableGeo();

    indexedUInt->setIndices(indexArray.p());
    m_curveDrawable->addPrimitiveSet(indexedUInt.p());

    cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray(m_curveVertices);
    m_curveDrawable->setVertexArray(vertexArray.p());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Riv3dWellLogCurveGeometryGenerator::clearCurvePointsAndGeometry()
{
    m_curveDrawable = nullptr;
    m_curveVertices.clear();
    m_curveMeasuredDepths.clear();
    m_curveValues.clear();
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
    float      closestDistance = m_planeWidth * 0.1;
    *closestPoint              = cvf::Vec3d::UNDEFINED;
    *measuredDepthAtPoint      = cvf::UNDEFINED_DOUBLE;
    *valueAtClosestPoint       = cvf::UNDEFINED_DOUBLE;
    if (m_curveVertices.size() < 2u) false;
    CVF_ASSERT(m_curveVertices.size() == m_curveValues.size());
    for (size_t i = 1; i < m_curveVertices.size(); ++i)
    {
        bool validCurveSegment = RigCurveDataTools::isValidValue(m_curveValues[i], false) &&
                                 RigCurveDataTools::isValidValue(m_curveValues[i - 1], false);
        if (validCurveSegment)
        {
            cvf::Vec3f a  = m_curveVertices[i - 1];
            cvf::Vec3f b  = m_curveVertices[i];
            cvf::Vec3f ap = globalIntersectionFloat - a;
            cvf::Vec3f ab = b - a;
            // Projected point is clamped to one of the end points of the segment.
            float      distanceToProjectedPointAlongAB = ap * ab / (ab * ab);
            float      clampedDistance                 = cvf::Math::clamp(distanceToProjectedPointAlongAB, 0.0f, 1.0f);
            cvf::Vec3f projectionOfGlobalIntersection  = a + clampedDistance * ab;
            float      distance                        = (projectionOfGlobalIntersection - globalIntersectionFloat).length();
            if (distance < closestDistance)
            {
                *closestPoint   = cvf::Vec3d(projectionOfGlobalIntersection);
                closestDistance = distance;
                *measuredDepthAtPoint =
                    m_curveMeasuredDepths[i - 1] * (1.0f - clampedDistance) + m_curveMeasuredDepths[i] * clampedDistance;
                *valueAtClosestPoint = m_curveValues[i - 1] * (1.0f - clampedDistance) + m_curveValues[i] * clampedDistance;
            }
        }
    }

    if (closestPoint->isUndefined()) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Riv3dWellLogCurveGeometryGenerator::createNewVerticesAlongTriangleEdges(const std::vector<cvf::Vec3f>& gridVertices)
{
    std::vector<cvf::Vec3f> expandedCurveVertices;
    std::vector<double> expandedMeasuredDepths;
    std::vector<double> expandedValues;
    size_t estimatedNumberOfPoints = m_curveVertices.size() + gridVertices.size();
    expandedCurveVertices.reserve(estimatedNumberOfPoints);
    expandedMeasuredDepths.reserve(estimatedNumberOfPoints);
    expandedValues.reserve(estimatedNumberOfPoints);
    for (size_t i = 0; i < m_curveVertices.size() - 1; i += 2)
    {
        if (RigCurveDataTools::isValidValue(m_curveValues[i], false) &&
            RigCurveDataTools::isValidValue(m_curveValues[i + 1], false))
        {
            expandedCurveVertices.push_back(m_curveVertices[i]);
            expandedMeasuredDepths.push_back(m_curveMeasuredDepths[i]);
            expandedValues.push_back(m_curveValues[i]);
            // Find segments that intersects the triangle edge
            caf::Line<float> curveLine(m_curveVertices[i], m_curveVertices[i + 1]);
            
            for (size_t j = 0; j < gridVertices.size() - 1; ++j)
            {
                caf::Line<float> gridLine(gridVertices[j], gridVertices[j + 1]);
                bool withinSegments = false;
                caf::Line<float> connectingLine = curveLine.findLineBetweenNearestPoints(gridLine, &withinSegments);
                
                if (withinSegments)
                {
                    cvf::Vec3f closestGridPoint = connectingLine.end();
                    double measuredDepth;
                    double valueAtPoint;
                    cvf::Vec3d closestPoint(closestGridPoint);
                    cvf::Vec3d dummyArgument;
                    // Interpolate measured depth and value
                    bool worked = findClosestPointOnCurve(closestPoint, &dummyArgument, &measuredDepth, &valueAtPoint);
                    if (worked)
                    {
                        expandedCurveVertices.push_back(closestGridPoint);
                        expandedMeasuredDepths.push_back(measuredDepth);
                        expandedValues.push_back(valueAtPoint);
                    }

                }
            }

            // Next original segment point
            expandedCurveVertices.push_back(m_curveVertices[i + 1]);
            expandedMeasuredDepths.push_back(m_curveMeasuredDepths[i + 1]);
            expandedValues.push_back(m_curveValues[i + 1]);
        }
    }

    m_curveVertices.swap(expandedCurveVertices);
    m_curveMeasuredDepths.swap(expandedMeasuredDepths);
    m_curveValues.swap(expandedValues);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Riv3dWellLogCurveGeometryGenerator::projectVerticesOntoTriangles(const std::vector<cvf::Vec3f>& gridVertices)
{
    for (size_t i = 0; i < m_curveVertices.size(); ++i)
    {
        for (size_t j = 0; j < gridVertices.size() - 2; j += 1)
        {
            cvf::Vec3f triangleVertex1, triangleVertex2, triangleVertex3;
            if (j % 2 == 0)
            {
                triangleVertex1 = gridVertices[j];
                triangleVertex2 = gridVertices[j + 1];
                triangleVertex3 = gridVertices[j + 2];
            }
            else
            {
                triangleVertex1 = gridVertices[j];
                triangleVertex2 = gridVertices[j + 2];
                triangleVertex3 = gridVertices[j + 1];
            }

            bool wasInsideTriangle = false;
            cvf::Vec3f projectedPoint = projectPointOntoTriangle(
                m_curveVertices[i], triangleVertex1, triangleVertex2, triangleVertex3, &wasInsideTriangle);
            if (wasInsideTriangle)
            {
                m_curveVertices[i] = projectedPoint;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3f Riv3dWellLogCurveGeometryGenerator::projectPointOntoTriangle(const cvf::Vec3f& point,
                                                                        const cvf::Vec3f& triangleVertex1,
                                                                        const cvf::Vec3f& triangleVertex2,
                                                                        const cvf::Vec3f& triangleVertex3,
                                                                        bool*             wasInsideTriangle)
{
    *wasInsideTriangle = false;
    cvf::Vec3f e1 = triangleVertex2 - triangleVertex1;
    cvf::Vec3f e2 = triangleVertex3 - triangleVertex1;
    cvf::Vec3f n = (e1.getNormalized() ^ e2.getNormalized()).getNormalized();

    // Project vertex onto triangle plane
    cvf::Vec3f av = point - triangleVertex1;
    cvf::Vec3f projectedPoint = point - (av * n) * n;

    // Calculate barycentric coordinates
    float areaABC = n * (e1 ^ e2);
    float areaPBC = n * ((triangleVertex2 - projectedPoint) ^ (triangleVertex3 - projectedPoint));
    float areaPCA = n * ((triangleVertex3 - projectedPoint) ^ (triangleVertex1 - projectedPoint));
    float u = areaPBC / areaABC;
    float v = areaPCA / areaABC;
    float w = 1.0 - u - v;

    if (u >= 0.0 && v >= 0.0 && w >= 0.0)
    {
        *wasInsideTriangle = true;
    }
    return projectedPoint;
}
