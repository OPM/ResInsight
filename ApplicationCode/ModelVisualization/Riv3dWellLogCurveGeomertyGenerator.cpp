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

#include "RimCase.h"
#include "RimGridView.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RigCurveDataTools.h"
#include "RigWellPath.h"

#include "cafDisplayCoordTransform.h"
#include "cvfPrimitiveSetIndexedUInt.h"

#include "cvfBoundingBox.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Riv3dWellLogCurveGeometryGenerator::Riv3dWellLogCurveGeometryGenerator(RimWellPath* wellPath, RimGridView* gridView)
    : m_wellPath(wellPath)
    , m_gridView(gridView)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo>
    Riv3dWellLogCurveGeometryGenerator::createCurveLine(const caf::DisplayCoordTransform* displayCoordTransform,
                                                        const cvf::BoundingBox&           wellPathClipBoundingBox,
                                                        const Rim3dWellLogCurve*          rim3dWellLogCurve) const
{
    std::vector<cvf::Vec3f> vertices;
    std::vector<cvf::uint>  indices;

    createCurveVerticesAndIndices(rim3dWellLogCurve, displayCoordTransform, wellPathClipBoundingBox, &vertices, &indices);

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
cvf::ref<cvf::DrawableGeo> Riv3dWellLogCurveGeometryGenerator::createGrid(const caf::DisplayCoordTransform* displayCoordTransform,
                                                                          const cvf::BoundingBox& wellPathClipBoundingBox,
                                                                          const Rim3dWellLogCurve::DrawPlane drawPlane,
                                                                          double gridIntervalSize) const
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
    double firstMd       = wellPathGeometry()->m_measuredDepths.at(newStartIndex);
    double lastMd        = wellPathGeometry()->m_measuredDepths.back();

    double md = lastMd;
    while (md >= firstMd)
    {
        cvf::Vec3d point = wellPathGeometry()->interpolatedPointAlongWellPath(md);
        gridPoints.push_back(point);
        md -= gridIntervalSize;
    }

    std::vector<cvf::Vec3d> pointNormals;

    std::vector<cvf::Vec3d> closestPoints;
    calculatePairsOfClosestPointsAlongWellPath(&closestPoints, gridPoints);

    pointNormals = calculateLineSegmentNormals(drawPlane, closestPoints, LINE_SEGMENTS);
    if (pointNormals.size() != gridPoints.size()) return nullptr;

    std::vector<cvf::Vec3f> vertices;
    vertices.reserve(gridPoints.size() * 2);

    std::vector<cvf::uint> indices;
    indices.reserve(gridPoints.size() * 2);

    cvf::uint indexCounter = 0;

    double offsetFromWellPathCenter = wellPathCenterToPlotStartOffset();

    // Normal lines
    for (size_t i = 0; i < pointNormals.size(); i++)
    {
        vertices.push_back(cvf::Vec3f(
            displayCoordTransform->transformToDisplayCoord(gridPoints[i] + pointNormals[i] * offsetFromWellPathCenter)));

        vertices.push_back(cvf::Vec3f(displayCoordTransform->transformToDisplayCoord(
            gridPoints[i] + pointNormals[i] * (offsetFromWellPathCenter + gridWidth()))));

        indices.push_back(indexCounter++);
        indices.push_back(indexCounter++);
    }

    // calculateLineSegmentNormals returns normals for the whole well path. Erase the part which is clipped off
    std::vector<cvf::Vec3d> wellPathSegmentNormals =
        calculateLineSegmentNormals(drawPlane, wellPathGeometry()->m_wellPathPoints, POLYLINE);
    wellPathSegmentNormals.erase(wellPathSegmentNormals.begin(), wellPathSegmentNormals.end() - wellPathPoints.size());

    // Line along and close to well
    for (size_t i = 0; i < wellPathPoints.size(); i++)
    {
        vertices.push_back(cvf::Vec3f(displayCoordTransform->transformToDisplayCoord(
            wellPathPoints[i] + wellPathSegmentNormals[i] * offsetFromWellPathCenter)));

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
            wellPathPoints[i] + wellPathSegmentNormals[i] * (offsetFromWellPathCenter + gridWidth()))));

        indices.push_back(indexCounter);
        indices.push_back(++indexCounter);
    }
    indices.pop_back();
    indices.pop_back();

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
void Riv3dWellLogCurveGeometryGenerator::createCurveVerticesAndIndices(const Rim3dWellLogCurve*          rim3dWellLogCurve,
                                                                       const caf::DisplayCoordTransform* displayCoordTransform,
                                                                       const cvf::BoundingBox&           wellPathClipBoundingBox,
                                                                       std::vector<cvf::Vec3f>*          vertices,
                                                                       std::vector<cvf::uint>*           indices) const
{
    if (!wellPathGeometry()) return;
    if (wellPathGeometry()->m_wellPathPoints.empty()) return;
    if (!wellPathClipBoundingBox.isValid()) return;

    std::vector<double> resultValues;
    std::vector<double> resultMds;
    rim3dWellLogCurve->curveValuesAndMds(&resultValues, &resultMds);

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

    if (interpolatedWellPathPoints.size() % 2 != 0)
    {
        interpolatedWellPathPoints.pop_back();
    }

    // Reverse list, since it was filled in the wrong order
    std::reverse(interpolatedWellPathPoints.begin(), interpolatedWellPathPoints.end());

    // Erase the result values for the part of the well which is clipped off, to match interpolatedWellPathPoints size
    resultValues.erase(resultValues.begin(), resultValues.end() - interpolatedWellPathPoints.size());

    std::vector<cvf::Vec3d> pairsOfWellPathPoints;
    calculatePairsOfClosestPointsAlongWellPath(&pairsOfWellPathPoints, interpolatedWellPathPoints);

    std::vector<cvf::Vec3d> pointNormals =
        calculateLineSegmentNormals(rim3dWellLogCurve->drawPlane(), pairsOfWellPathPoints, LINE_SEGMENTS);
    if (interpolatedWellPathPoints.size() != pointNormals.size()) return;

    double maxResult = -HUGE_VAL;
    double minResult = HUGE_VAL;

    for (double result : resultValues)
    {
        if (!RigCurveDataTools::isValidValue(result, false)) continue;

        maxResult = std::max(result, maxResult);
        minResult = std::min(result, minResult);
    }

    vertices->resize(interpolatedWellPathPoints.size());

    double plotRangeToResultRangeFactor = gridWidth() / (maxResult - minResult);
    double offsetFromWellPathCenter     = wellPathCenterToPlotStartOffset();

    for (size_t i = 0; i < pointNormals.size(); i++)
    {
        double scaledResult = 0;

        if (RigCurveDataTools::isValidValue(resultValues[i], false))
        {
            scaledResult = offsetFromWellPathCenter + (resultValues[i] - minResult) * plotRangeToResultRangeFactor;
        }

        (*vertices)[i] = cvf::Vec3f(
            displayCoordTransform->transformToDisplayCoord(interpolatedWellPathPoints[i] + scaledResult * pointNormals[i]));
    }

    std::vector<std::pair<size_t, size_t>> valuesIntervals =
        RigCurveDataTools::calculateIntervalsOfValidValues(resultValues, false);

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
std::vector<cvf::Vec3d> Riv3dWellLogCurveGeometryGenerator::calculateLineSegmentNormals(Rim3dWellLogCurve::DrawPlane   drawPlane,
                                                                                        const std::vector<cvf::Vec3d>& vertices,
                                                                                        VertexOrganization organization) const
{
    std::vector<cvf::Vec3d> pointNormals;

    if (!wellPathGeometry()) return pointNormals;
    if (vertices.empty()) return pointNormals;

    const cvf::Vec3d globalDirection =
        (wellPathGeometry()->m_wellPathPoints.back() - wellPathGeometry()->m_wellPathPoints.front()).getNormalized();

    const cvf::Vec3d up(0, 0, 1);

    size_t intervalSize;
    if (organization == LINE_SEGMENTS)
    {
        pointNormals.reserve(vertices.size() / 2);
        intervalSize = 2;
    }
    else // organization == POLYLINE
    {
        pointNormals.reserve(vertices.size());
        intervalSize = 1;
    }

    cvf::Vec3d normal;

    for (size_t i = 0; i < vertices.size() - 1; i += intervalSize)
    {
        cvf::Vec3d p1 = vertices[i];
        cvf::Vec3d p2 = vertices[i + 1];

        cvf::Vec3d vecAlongPath = (p2 - p1).getNormalized();

        double dotProduct = up * vecAlongPath;

        cvf::Vec3d Ex;

        if (cvf::Math::abs(dotProduct) > 0.7071)
        {
            Ex = globalDirection;
        }
        else
        {
            Ex = vecAlongPath;
        }

        cvf::Vec3d Ey = (up ^ Ex).getNormalized();
        cvf::Vec3d Ez = (Ex ^ Ey).getNormalized();

        switch (drawPlane)
        {
            case Rim3dWellLogCurve::HORIZONTAL_LEFT:
                normal = -Ey;
                break;
            case Rim3dWellLogCurve::HORIZONTAL_RIGHT:
                normal = Ey;
                break;
            case Rim3dWellLogCurve::VERTICAL_ABOVE:
                normal = Ez;
                break;
            case Rim3dWellLogCurve::VERTICAL_BELOW:
                normal = -Ez;
                break;
            default:
                break;
        }

        pointNormals.push_back(normal);
    }

    if (organization == POLYLINE)
    {
        pointNormals.push_back(normal);
    }

    return pointNormals;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double Riv3dWellLogCurveGeometryGenerator::wellPathCenterToPlotStartOffset() const
{
    if (!m_gridView) return 0;

    double cellSize = m_gridView->ownerCase()->characteristicCellSize();

    return m_wellPath->wellPathRadius(cellSize) * 2;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double Riv3dWellLogCurveGeometryGenerator::gridWidth() const
{
    if (!m_gridView) return 0;

    double cellSize = m_gridView->ownerCase()->characteristicCellSize();

    return m_wellPath->wellPathRadius(cellSize) * 3;
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
void Riv3dWellLogCurveGeometryGenerator::calculatePairsOfClosestPointsAlongWellPath(
    std::vector<cvf::Vec3d>* closestWellPathPoints,
    std::vector<cvf::Vec3d>& points) const
{
    CVF_ASSERT(closestWellPathPoints != nullptr);

    for (const cvf::Vec3d point : points)
    {
        cvf::Vec3d p1 = cvf::Vec3d::UNDEFINED;
        cvf::Vec3d p2 = cvf::Vec3d::UNDEFINED;
        wellPathGeometry()->twoClosestPoints(point, &p1, &p2);
        if (p1.isUndefined() || p2.isUndefined()) continue;

        closestWellPathPoints->push_back(p1);
        closestWellPathPoints->push_back(p2);
    }
}
