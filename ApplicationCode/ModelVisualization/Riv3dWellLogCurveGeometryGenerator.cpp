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
                                                              const std::vector<cvf::Vec3f>&    drawSurfaceVertices)
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

    double plotRangeToResultRangeFactor = planeWidth / (maxClampedResult - minClampedResult);

    m_curveVertices.reserve(interpolatedWellPathPoints.size());
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
    m_curveVertices = projectVerticesOntoTriangles(m_curveVertices, drawSurfaceVertices);
    
    m_bottomVertices.reserve(m_curveVertices.size() + 2);
    for (size_t i = 0; i < m_curveVertices.size(); ++i)
    {
        double md = m_curveMeasuredDepths[i];
        cvf::Vec3d point = wellPathGeometry()->interpolatedVectorAlongWellPath(wellPathPoints, md);
        cvf::Vec3d normal = wellPathGeometry()->interpolatedVectorAlongWellPath(wellPathCurveNormals, md);
        point += planeOffsetFromWellPathCenter * normal;
        m_bottomVertices.push_back(cvf::Vec3f(point));
    }
    m_bottomVertices = projectVerticesOntoTriangles(m_bottomVertices, drawSurfaceVertices);

    createNewVerticesAlongTriangleEdges(drawSurfaceVertices);

    {
        std::vector<cvf::uint> indices;
        indices.reserve(m_curveVertices.size() * 2);
        for (size_t i = 0; i < m_curveVertices.size() - 1; ++i)
        {
            indices.push_back(cvf::uint(i));
            indices.push_back(cvf::uint(i + 1));
        }

        cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUInt = new cvf::PrimitiveSetIndexedUInt(cvf::PrimitiveType::PT_LINES);
        cvf::ref<cvf::UIntArray>               indexArray = new cvf::UIntArray(indices);

        m_curveDrawable = new cvf::DrawableGeo();

        indexedUInt->setIndices(indexArray.p());
        m_curveDrawable->addPrimitiveSet(indexedUInt.p());

        cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray(m_curveVertices);
        m_curveDrawable->setVertexArray(vertexArray.p());
    }

    {
        CVF_ASSERT(m_bottomVertices.size() == m_curveVertices.size());
        cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray(m_bottomVertices.size() + m_curveVertices.size());
        for (size_t i = 0; i < m_bottomVertices.size(); ++i)
        {
            (*vertexArray)[2 * i] = m_bottomVertices[i];
            (*vertexArray)[2 * i + 1] = m_curveVertices[i];
        }
        
        std::vector<cvf::uint> indices;
        indices.reserve(vertexArray->size());
        for (size_t i = 0; i < vertexArray->size(); ++i)
        {
            indices.push_back(cvf::uint(i));
        }

        cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUInt = new cvf::PrimitiveSetIndexedUInt(cvf::PrimitiveType::PT_TRIANGLE_STRIP);
        cvf::ref<cvf::UIntArray>               indexArray = new cvf::UIntArray(indices);

        m_curveFilledDrawable = new cvf::DrawableGeo();
        indexedUInt->setIndices(indexArray.p());
        m_curveFilledDrawable->addPrimitiveSet(indexedUInt.p());
        m_curveFilledDrawable->setVertexArray(vertexArray.p());
    }

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Riv3dWellLogCurveGeometryGenerator::clearCurvePointsAndGeometry()
{
    m_curveDrawable = nullptr;
    m_curveFilledDrawable = nullptr;
    m_curveVertices.clear();
    m_bottomVertices.clear();
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
cvf::ref<cvf::DrawableGeo> Riv3dWellLogCurveGeometryGenerator::curveFilledDrawable()
{
    return m_curveFilledDrawable;
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
void Riv3dWellLogCurveGeometryGenerator::createNewVerticesAlongTriangleEdges(const std::vector<cvf::Vec3f>& drawSurfaceVertices)
{
    std::vector<cvf::Vec3f> expandedCurveVertices;
    std::vector<cvf::Vec3f> expandedBottomVertices;
    std::vector<double>     expandedMeasuredDepths;
    std::vector<double>     expandedValues;
    size_t                  estimatedNumberOfPoints = m_curveVertices.size() + drawSurfaceVertices.size();
    expandedCurveVertices.reserve(estimatedNumberOfPoints);
    expandedBottomVertices.reserve(estimatedNumberOfPoints);
    expandedMeasuredDepths.reserve(estimatedNumberOfPoints);
    expandedValues.reserve(estimatedNumberOfPoints);

    for (size_t i = 0; i < m_curveVertices.size() - 1; i += 2)
    {
        if (RigCurveDataTools::isValidValue(m_curveValues[i], false) &&
            RigCurveDataTools::isValidValue(m_curveValues[i + 1], false))
        {
            cvf::Vec3f lastVertex        = m_curveVertices[i];
            cvf::Vec3f fullSegmentVector = m_curveVertices[i + 1] - m_curveVertices[i];

            std::vector<cvf::Vec3f> extraVertices;
            std::vector<cvf::Vec3f> extraBottomVertices;

            createNewVerticesAlongSegment(m_curveVertices[i],
                                          m_curveVertices[i + 1],
                                          drawSurfaceVertices,
                                          &extraVertices,
                                          &m_bottomVertices[i],
                                          &m_bottomVertices[i + 1],
                                          &extraBottomVertices);

            CVF_ASSERT(extraVertices.size() == extraBottomVertices.size());

            for (const cvf::Vec3f& extraVertex : extraVertices)
            {
                cvf::Vec3f newSegmentVector = extraVertex - lastVertex;
                // Scalar projection (a * b / |b|) divided by full segment length to become (a * b / |b|^2)
                float dotProduct               = newSegmentVector * fullSegmentVector;
                float fractionAlongFullSegment = dotProduct / fullSegmentVector.lengthSquared();
                float measuredDepth            = m_curveMeasuredDepths[i] * (1 - fractionAlongFullSegment) +
                                      m_curveMeasuredDepths[i + 1] * fractionAlongFullSegment;
                float valueAtPoint =
                    m_curveValues[i] * (1 - fractionAlongFullSegment) + m_curveValues[i + 1] * fractionAlongFullSegment;
                expandedCurveVertices.push_back(extraVertex);
                expandedMeasuredDepths.push_back(measuredDepth);
                expandedValues.push_back(valueAtPoint);
                lastVertex = extraVertex;
            }
            expandedBottomVertices.insert(expandedBottomVertices.end(), extraBottomVertices.begin(), extraBottomVertices.end());
        }
    }

    CVF_ASSERT(expandedCurveVertices.size() == expandedBottomVertices.size());
    m_curveVertices.swap(expandedCurveVertices);
    m_bottomVertices.swap(expandedBottomVertices);
    m_curveMeasuredDepths.swap(expandedMeasuredDepths);
    m_curveValues.swap(expandedValues);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void Riv3dWellLogCurveGeometryGenerator::createNewVerticesAlongSegment(const cvf::Vec3f&              ptStart,
                                                                       const cvf::Vec3f&              ptEnd,
                                                                       const std::vector<cvf::Vec3f>& drawSurfaceVertices,
                                                                       std::vector<cvf::Vec3f>*       extraVertices,
                                                                       const cvf::Vec3f*              ptBottomStart,
                                                                       const cvf::Vec3f*              ptBottomEnd,
                                                                       std::vector<cvf::Vec3f>*       extraBottomVertices)
{
    cvf::Vec3f fullSegmentVector = ptEnd - ptStart;
    extraVertices->push_back(ptStart);

    cvf::Vec3f fullBottomVector;
    if (ptBottomStart && ptBottomEnd && extraBottomVertices)
    {
        fullBottomVector = *ptBottomEnd - *ptBottomStart;
        extraBottomVertices->push_back(*ptBottomStart);
    }

    // Find segments that intersects the triangle edges
    for (size_t j = 0; j < drawSurfaceVertices.size() - 2; j += 1)
    {
        caf::Line<float> triangleEdge1 = caf::Line<float>(drawSurfaceVertices[j], drawSurfaceVertices[j + 1]);
        caf::Line<float> triangleEdge2 = caf::Line<float>(drawSurfaceVertices[j + 2], drawSurfaceVertices[j + 1]);
        cvf::Vec3f       triangleNormal =
            (triangleEdge1.vector().getNormalized() ^ triangleEdge2.vector().getNormalized()).getNormalized();

        cvf::Vec3f       currentSubSegment      = ptEnd - extraVertices->back();
        cvf::Vec3f       projectedSegmentVector = currentSubSegment - (currentSubSegment * triangleNormal) * triangleNormal;
        caf::Line<float> projectedCurveLine(extraVertices->back(), extraVertices->back() + projectedSegmentVector);

        // Only attempt to find intersections with the first edge. The other edge is handled with the next triangle.
        bool             withinSegments = false;
        caf::Line<float> connectingLine = projectedCurveLine.findLineBetweenNearestPoints(triangleEdge1, &withinSegments);

        cvf::Vec3f newVertex        = connectingLine.end();
        cvf::Vec3f newSegmentVector = newVertex - extraVertices->back();
        if (withinSegments && newSegmentVector.lengthSquared() < fullSegmentVector.lengthSquared())
        {
            extraVertices->push_back(newVertex);

            if (ptBottomStart && ptBottomEnd && extraBottomVertices)
            {
                // Do the same for the bottom line, however we need to ensure we add the same amount of points.
                cvf::Vec3f currentBottomSegment = *ptBottomEnd - extraBottomVertices->back();
                cvf::Vec3f projectedBottomVector =
                    currentBottomSegment - (currentBottomSegment * triangleNormal) * triangleNormal;
                caf::Line<float> projectedBottomLine(extraBottomVertices->back(),
                                                     extraBottomVertices->back() + projectedBottomVector);
                bool             withinBottomSegments = false;

                caf::Line<float> bottomConnectingLine =
                    projectedBottomLine.findLineBetweenNearestPoints(triangleEdge1, &withinBottomSegments);
                cvf::Vec3f newBottomVertex = bottomConnectingLine.end();
                cvf::Vec3f newBottomVector = newBottomVertex - extraBottomVertices->back();
                if (!(withinBottomSegments && newBottomVector.lengthSquared() < fullBottomVector.lengthSquared()))
                {
                    newBottomVertex = extraBottomVertices->back();
                }
                extraBottomVertices->push_back(newBottomVertex);
            }
        }
    }
    extraVertices->push_back(ptEnd);
    if (ptBottomStart && ptBottomEnd && extraBottomVertices)
    {
        extraBottomVertices->push_back(*ptBottomEnd);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3f>
    Riv3dWellLogCurveGeometryGenerator::projectVerticesOntoTriangles(const std::vector<cvf::Vec3f>& originalVertices,
                                                                     const std::vector<cvf::Vec3f>& drawSurfaceVertices)
{
    std::vector<cvf::Vec3f> projectedVertices;
    projectedVertices.reserve(originalVertices.size());
    for (size_t i = 0; i < originalVertices.size(); ++i)
    {
        // Sort projections onto triangle by the distance of the projection.
        std::map<float, cvf::Vec3f> projectionsInsideTriangle;
        for (size_t j = 0; j < drawSurfaceVertices.size() - 2; j += 1)
        {
            cvf::Vec3f triangleVertex1, triangleVertex2, triangleVertex3;
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
            cvf::Vec3f projectedPoint    = projectPointOntoTriangle(
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
cvf::Vec3f Riv3dWellLogCurveGeometryGenerator::projectPointOntoTriangle(const cvf::Vec3f& point,
                                                                        const cvf::Vec3f& triangleVertex1,
                                                                        const cvf::Vec3f& triangleVertex2,
                                                                        const cvf::Vec3f& triangleVertex3,
                                                                        bool*             wasInsideTriangle)
{
    *wasInsideTriangle = false;
    cvf::Vec3f e1      = triangleVertex2 - triangleVertex1;
    cvf::Vec3f e2      = triangleVertex3 - triangleVertex1;
    cvf::Vec3f n       = (e1.getNormalized() ^ e2.getNormalized()).getNormalized();

    // Project vertex onto triangle plane
    cvf::Vec3f av             = point - triangleVertex1;
    cvf::Vec3f projectedAv    = av - (av * n) * n;
    cvf::Vec3f projectedPoint = projectedAv + triangleVertex1;

    // Calculate barycentric coordinates
    float areaABC = n * (e1 ^ e2);
    float areaPBC = n * ((triangleVertex2 - projectedPoint) ^ (triangleVertex3 - projectedPoint));
    float areaPCA = n * ((triangleVertex3 - projectedPoint) ^ (triangleVertex1 - projectedPoint));
    float u       = areaPBC / areaABC;
    float v       = areaPCA / areaABC;
    float w       = 1.0 - u - v;

    if (u >= -1.0e-6 && v >= -1.0e-6 && w >= -1.0e-6)
    {
        *wasInsideTriangle = true;
        // Clamp to ensure it is inside the triangle
        u              = cvf::Math::clamp(u, 0.0f, 1.0f);
        v              = cvf::Math::clamp(v, 0.0f, 1.0f);
        w              = cvf::Math::clamp(w, 0.0f, 1.0f);
        projectedPoint = triangleVertex1 * u + triangleVertex2 * v + triangleVertex3 * w;
    }
    return projectedPoint;
}
