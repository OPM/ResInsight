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

#include "Riv3dWellLogCurveGeometryGenerator.h"

#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RiaCurveDataTools.h"
#include "RigWellPath.h"
#include "RigWellPathGeometryTools.h"

#include "cafLine.h"
#include "cafDisplayCoordTransform.h"
#include "cvfPrimitiveSetIndexedUInt.h"

#include "cvfBoundingBox.h"
#include "cvfMath.h"

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
                                                              const std::vector<cvf::Vec3d>&    drawSurfaceVertices)
{
    CVF_ASSERT(rim3dWellLogCurve);

    // Make sure all drawables are cleared in case we return early to avoid a
    // previous drawable being "stuck" when changing result type.
    clearCurvePointsAndGeometry();
    
    float curveUIRange = rim3dWellLogCurve->maxCurveUIValue() - rim3dWellLogCurve->minCurveUIValue();
    if (curveUIRange < 1.0e-6f)
    {
        return;
    }


    std::vector<double> resultValues;
    std::vector<double> resultMds;
    rim3dWellLogCurve->curveValuesAndMds(&resultValues, &resultMds);

    m_planeWidth = planeWidth;

    if (!wellPathGeometry()) return;
    if (wellPathGeometry()->m_wellPathPoints.empty()) return;
    if (!wellPathClipBoundingBox.isValid()) return;

    if (resultValues.empty()) return;
    CVF_ASSERT(resultValues.size() == resultMds.size());

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

    double maxVisibleResult = -std::numeric_limits<double>::max();
    double minVisibleResult = std::numeric_limits<double>::max();

    double minCurveValue = rim3dWellLogCurve->minCurveUIValue();
    double maxCurveValue = rim3dWellLogCurve->maxCurveUIValue();

    double curveEpsilon = 1.0e-6;

    for (double& result : m_curveValues)
    {
        if (!RiaCurveDataTools::isValidValue(result, false)) continue;

        if ((minCurveValue - result) > curveEpsilon * curveUIRange)
        {
            result = minCurveValue - curveEpsilon;
        }
        else if ((result - maxCurveValue) > curveEpsilon * curveUIRange)
        {
            result = maxCurveValue + curveEpsilon;
        }
        else
        {
            maxVisibleResult = std::max(result, maxVisibleResult);
            minVisibleResult = std::min(result, minVisibleResult);
        }
    }

    if (minVisibleResult > maxVisibleResult)
    {
        return;
    }

    double plotRangeToResultRangeFactor = planeWidth / curveUIRange;

    m_curveVertices.reserve(interpolatedWellPathPoints.size());
    for (size_t i = 0; i < interpolatedWellPathPoints.size(); i++)
    {
        double scaledResult = 0;

        if (RiaCurveDataTools::isValidValue(m_curveValues[i], false))
        {
            scaledResult = planeOffsetFromWellPathCenter + (m_curveValues[i] - minCurveValue) * plotRangeToResultRangeFactor;
        }
        cvf::Vec3d curvePoint(interpolatedWellPathPoints[i] + scaledResult * interpolatedCurveNormals[i]);
        m_curveVertices.push_back(curvePoint);
    }
    m_curveVertices = projectVerticesOntoTriangles(m_curveVertices, drawSurfaceVertices);
    
    createNewVerticesAlongTriangleEdges(drawSurfaceVertices);

    {
        std::vector<cvf::uint> indices;
        indices.reserve(m_curveVertices.size() * 2);
        for (size_t i = 0; i < m_curveVertices.size() - 1; ++i)
        {
            if (RiaCurveDataTools::isValidValue(m_curveValues[i], false) &&
                RiaCurveDataTools::isValidValue(m_curveValues[i + 1], false))
            {
                if (cvf::Math::valueInRange(m_curveValues[i], minCurveValue, maxCurveValue) ||
                    cvf::Math::valueInRange(m_curveValues[i + 1], minCurveValue, maxCurveValue))
                {
                    indices.push_back(cvf::uint(i));
                    indices.push_back(cvf::uint(i + 1));
                }
            }
        }

        cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUInt = new cvf::PrimitiveSetIndexedUInt(cvf::PrimitiveType::PT_LINES);
        cvf::ref<cvf::UIntArray>               indexArray = new cvf::UIntArray(indices);

        m_curveDrawable = new cvf::DrawableGeo();

        indexedUInt->setIndices(indexArray.p());
        m_curveDrawable->addPrimitiveSet(indexedUInt.p());

        cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray(m_curveVertices.size());
        for (size_t i = 0; i < m_curveVertices.size(); ++i)
        {
            (*vertexArray)[i] = cvf::Vec3f(m_curveVertices[i]);
        }
        m_curveDrawable->setVertexArray(vertexArray.p());
    }
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
    double      closestDistance = m_planeWidth * 0.1;
    *closestPoint              = cvf::Vec3d::UNDEFINED;
    *measuredDepthAtPoint      = cvf::UNDEFINED_DOUBLE;
    *valueAtClosestPoint       = cvf::UNDEFINED_DOUBLE;
    if (m_curveVertices.size() < 2u) false;
    CVF_ASSERT(m_curveVertices.size() == m_curveValues.size());
    for (size_t i = 1; i < m_curveVertices.size(); ++i)
    {
        bool validCurveSegment = RiaCurveDataTools::isValidValue(m_curveValues[i], false) &&
                                 RiaCurveDataTools::isValidValue(m_curveValues[i - 1], false);
        if (validCurveSegment)
        {
            cvf::Vec3d a  = m_curveVertices[i - 1];
            cvf::Vec3d b  = m_curveVertices[i];
            cvf::Vec3d ap = globalIntersection - a;
            cvf::Vec3d ab = b - a;
            // Projected point is clamped to one of the end points of the segment.
            double      distanceToProjectedPointAlongAB = ap * ab / (ab * ab);
            double      clampedDistance                 = cvf::Math::clamp(distanceToProjectedPointAlongAB, 0.0, 1.0);
            cvf::Vec3d projectionOfGlobalIntersection  = a + clampedDistance * ab;
            double      distance                        = (projectionOfGlobalIntersection - globalIntersection).length();
            if (distance < closestDistance)
            {
                *closestPoint   = cvf::Vec3d(projectionOfGlobalIntersection);
                closestDistance = distance;
                *measuredDepthAtPoint =
                    m_curveMeasuredDepths[i - 1] * (1.0 - clampedDistance) + m_curveMeasuredDepths[i] * clampedDistance;
                *valueAtClosestPoint = m_curveValues[i - 1] * (1.0 - clampedDistance) + m_curveValues[i] * clampedDistance;
            }
        }
    }

    if (closestPoint->isUndefined()) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Riv3dWellLogCurveGeometryGenerator::createNewVerticesAlongTriangleEdges(const std::vector<cvf::Vec3d>& drawSurfaceVertices)
{
    std::vector<cvf::Vec3d> expandedCurveVertices;
    std::vector<double>     expandedMeasuredDepths;
    std::vector<double>     expandedValues;
    size_t                  estimatedNumberOfPoints = m_curveVertices.size() + drawSurfaceVertices.size();
    expandedCurveVertices.reserve(estimatedNumberOfPoints);
    expandedMeasuredDepths.reserve(estimatedNumberOfPoints);
    expandedValues.reserve(estimatedNumberOfPoints);

    for (size_t i = 0; i < m_curveVertices.size() - 1; i += 2)
    {
        if (RiaCurveDataTools::isValidValue(m_curveValues[i], false) &&
            RiaCurveDataTools::isValidValue(m_curveValues[i + 1], false))
        {
            cvf::Vec3d lastVertex        = m_curveVertices[i];
            cvf::Vec3d fullSegmentVector = m_curveVertices[i + 1] - m_curveVertices[i];

            std::vector<cvf::Vec3d> extraVertices;

            createNewVerticesAlongSegment(m_curveVertices[i],
                                          m_curveVertices[i + 1],
                                          drawSurfaceVertices,
                                          &extraVertices);

            for (const cvf::Vec3d& extraVertex : extraVertices)
            {
                cvf::Vec3d newSegmentVector = extraVertex - lastVertex;
                // Scalar projection (a * b / |b|) divided by full segment length to become (a * b / |b|^2)
                double dotProduct               = newSegmentVector * fullSegmentVector;
                double fractionAlongFullSegment = dotProduct / fullSegmentVector.lengthSquared();
                double measuredDepth            = m_curveMeasuredDepths[i] * (1 - fractionAlongFullSegment) +
                                                  m_curveMeasuredDepths[i + 1] * fractionAlongFullSegment;
                double valueAtPoint =
                    m_curveValues[i] * (1 - fractionAlongFullSegment) + m_curveValues[i + 1] * fractionAlongFullSegment;
                expandedCurveVertices.push_back(extraVertex);
                expandedMeasuredDepths.push_back(measuredDepth);
                expandedValues.push_back(valueAtPoint);
                lastVertex = extraVertex;
            }
        }
        else
        {
            // Add the invalid points and values.
            expandedCurveVertices.push_back(m_curveVertices[i]);
            expandedMeasuredDepths.push_back(m_curveMeasuredDepths[i]);
            expandedValues.push_back(m_curveValues[i]);
            
        }
    }

    m_curveVertices.swap(expandedCurveVertices);
    m_curveMeasuredDepths.swap(expandedMeasuredDepths);
    m_curveValues.swap(expandedValues);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Riv3dWellLogCurveGeometryGenerator::createNewVerticesAlongSegment(const cvf::Vec3d&              ptStart,
                                                                       const cvf::Vec3d&              ptEnd,
                                                                       const std::vector<cvf::Vec3d>& drawSurfaceVertices,
                                                                       std::vector<cvf::Vec3d>*       extraVertices)
{
    cvf::Vec3d fullSegmentVector = ptEnd - ptStart;
    extraVertices->push_back(ptStart);

    // Find segments that intersects the triangle edges
    for (size_t j = 0; j < drawSurfaceVertices.size() - 2; j += 1)
    {
        caf::Line<double> triangleEdge1 = caf::Line<double>(drawSurfaceVertices[j], drawSurfaceVertices[j + 1]);
        caf::Line<double> triangleEdge2 = caf::Line<double>(drawSurfaceVertices[j + 2], drawSurfaceVertices[j + 1]);
        cvf::Vec3d       triangleNormal =
            (triangleEdge1.vector().getNormalized() ^ triangleEdge2.vector().getNormalized()).getNormalized();

        cvf::Vec3d       currentSubSegment      = ptEnd - extraVertices->back();
        cvf::Vec3d       projectedSegmentVector = currentSubSegment - (currentSubSegment * triangleNormal) * triangleNormal;
        caf::Line<double> projectedCurveLine(extraVertices->back(), extraVertices->back() + projectedSegmentVector);

        // Only attempt to find intersections with the first edge. The other edge is handled with the next triangle.
        bool withinSegments = false;
        caf::Line<double> connectingLine = projectedCurveLine.findLineBetweenNearestPoints(triangleEdge1, &withinSegments);

        cvf::Vec3d newVertex        = connectingLine.end();
        cvf::Vec3d newSegmentVector = newVertex - extraVertices->back();
        if (withinSegments && newSegmentVector.lengthSquared() < currentSubSegment.lengthSquared())
        {
            extraVertices->push_back(newVertex);
        }
    }
    extraVertices->push_back(ptEnd);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d>
    Riv3dWellLogCurveGeometryGenerator::projectVerticesOntoTriangles(const std::vector<cvf::Vec3d>& originalVertices,
                                                                     const std::vector<cvf::Vec3d>& drawSurfaceVertices)
{
    std::vector<cvf::Vec3d> projectedVertices;
    projectedVertices.reserve(originalVertices.size());
    for (size_t i = 0; i < originalVertices.size(); ++i)
    {
        // Sort projections onto triangle by the distance of the projection.
        std::map<double, cvf::Vec3d> projectionsInsideTriangle;
        for (size_t j = 0; j < drawSurfaceVertices.size() - 2; j += 1)
        {
            cvf::Vec3d triangleVertex1, triangleVertex2, triangleVertex3;
            if (j % 2 == 0)
            {
                triangleVertex1 = drawSurfaceVertices[j];
                triangleVertex2 = drawSurfaceVertices[j + 1];
                triangleVertex3 = drawSurfaceVertices[j + 2];
            }
            else
            {
                triangleVertex1 = drawSurfaceVertices[j];
                triangleVertex2 = drawSurfaceVertices[j + 2];
                triangleVertex3 = drawSurfaceVertices[j + 1];
            }

            bool       wasInsideTriangle = false;
            cvf::Vec3d projectedPoint    = projectPointOntoTriangle(
                originalVertices[i], triangleVertex1, triangleVertex2, triangleVertex3, &wasInsideTriangle);
            if (wasInsideTriangle)
            {
                projectionsInsideTriangle.insert(
                    std::make_pair((projectedPoint - originalVertices[i]).lengthSquared(), projectedPoint));
            }
        }

        // Take the closest projection
        if (!projectionsInsideTriangle.empty())
        {
            projectedVertices.push_back(projectionsInsideTriangle.begin()->second);
        }
        else
        {
            projectedVertices.push_back(originalVertices[i]);
        }
    }
    return projectedVertices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d Riv3dWellLogCurveGeometryGenerator::projectPointOntoTriangle(const cvf::Vec3d& point,
                                                                        const cvf::Vec3d& triangleVertex1,
                                                                        const cvf::Vec3d& triangleVertex2,
                                                                        const cvf::Vec3d& triangleVertex3,
                                                                        bool*             wasInsideTriangle)
{
    *wasInsideTriangle = false;
    cvf::Vec3d e1      = triangleVertex2 - triangleVertex1;
    cvf::Vec3d e2      = triangleVertex3 - triangleVertex1;
    cvf::Vec3d n       = (e1.getNormalized() ^ e2.getNormalized()).getNormalized();

    // Project vertex onto triangle plane
    cvf::Vec3d av             = point - triangleVertex1;
    cvf::Vec3d projectedAv    = av - (av * n) * n;
    cvf::Vec3d projectedPoint = projectedAv + triangleVertex1;

    // Calculate barycentric coordinates
    double areaABC = n * (e1 ^ e2);
    double areaPBC = n * ((triangleVertex2 - projectedPoint) ^ (triangleVertex3 - projectedPoint));
    double areaPCA = n * ((triangleVertex3 - projectedPoint) ^ (triangleVertex1 - projectedPoint));
    double u       = areaPBC / areaABC;
    double v       = areaPCA / areaABC;
    double w       = 1.0 - u - v;

    if (u >= -1.0e-6 && v >= -1.0e-6 && w >= -1.0e-6)
    {
        *wasInsideTriangle = true;
        // Clamp to ensure it is inside the triangle
        u              = cvf::Math::clamp(u, 0.0, 1.0);
        v              = cvf::Math::clamp(v, 0.0, 1.0);
        w              = cvf::Math::clamp(w, 0.0, 1.0);
        projectedPoint = triangleVertex1 * u + triangleVertex2 * v + triangleVertex3 * w;
    }
    return projectedPoint;
}
