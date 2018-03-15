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

#include "RigCurveDataTools.h"
#include "RigWellPath.h"

#include "cafDisplayCoordTransform.h"
#include "cvfPrimitiveSetIndexedUInt.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo>
    Riv3dWellLogCurveGeometryGenerator::createCurveLine(const caf::DisplayCoordTransform* displayCoordTransform,
                                                        const Rim3dWellLogCurve*          rim3dWellLogCurve) const
{
    std::vector<cvf::Vec3f> vertices;
    std::vector<cvf::uint>  indices;

    createCurveVerticesAndIndices(rim3dWellLogCurve, displayCoordTransform, &vertices, &indices);

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
                                                                          const Rim3dWellLogCurve* rim3dWellLogCurve) const
{
    std::vector<cvf::Vec3d> wellPathPoints = m_wellPathGeometry->m_wellPathPoints;

    cvf::Vec3d globalDirection = (wellPathPoints.back() - wellPathPoints.front()).getNormalized();

    std::vector<cvf::Vec3d> pointNormals = calculatePointNormals(rim3dWellLogCurve->drawPlane(), wellPathPoints);

    std::vector<cvf::Vec3f> vertices;
    vertices.reserve(wellPathPoints.size());

    std::vector<cvf::uint> indices;
    indices.reserve(wellPathPoints.size());

    cvf::uint counter = 0;

    for (size_t i = 0; i < pointNormals.size(); i += 2)
    {
        vertices.push_back(cvf::Vec3f(displayCoordTransform->transformToDisplayCoord(wellPathPoints[i])));
        vertices.push_back(cvf::Vec3f(displayCoordTransform->transformToDisplayCoord(wellPathPoints[i] + pointNormals[i] * 100)));

        indices.push_back(counter++);
        indices.push_back(counter++);
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
void Riv3dWellLogCurveGeometryGenerator::createCurveVerticesAndIndices(const Rim3dWellLogCurve*          rim3dWellLogCurve,
                                                                       const caf::DisplayCoordTransform* displayCoordTransform,
                                                                       std::vector<cvf::Vec3f>*          vertices,
                                                                       std::vector<cvf::uint>*           indices) const
{
    if (m_wellPathGeometry.isNull()) return;

    std::vector<double> resultValues;
    std::vector<double> mds;
    rim3dWellLogCurve->curveValuesAndMds(&resultValues, &mds);

    if (resultValues.empty()) return;
    CVF_ASSERT(resultValues.size() == mds.size());

    cvf::Vec3d globalDirection =
        (m_wellPathGeometry->m_wellPathPoints.back() - m_wellPathGeometry->m_wellPathPoints.front()).getNormalized();

    std::vector<cvf::Vec3d> interpolatedWellPathPoints;
    interpolatedWellPathPoints.reserve(mds.size());

    for (double md : mds)
    {
        interpolatedWellPathPoints.push_back(m_wellPathGeometry->interpolatedPointAlongWellPath(md));
    }

    std::vector<cvf::Vec3d> pointNormals = calculatePointNormals(rim3dWellLogCurve->drawPlane(), interpolatedWellPathPoints);

    double maxResult = -HUGE_VAL;
    double minResult = HUGE_VAL;

    for (double result : resultValues)
    {
        if (!RigCurveDataTools::isValidValue(result, false)) continue;

        maxResult = std::max(result, maxResult);
        minResult = std::min(result, minResult);
    }

    vertices->resize(interpolatedWellPathPoints.size());

    double range  = maxResult - minResult;
    double factor = 60.0 / range;
    double offset = 30.0;

    if (minResult < 0)
    {
        offset += cvf::Math::abs(minResult * factor);
    }

    for (size_t i = 0; i < pointNormals.size(); i++)
    {
        cvf::Vec3d result(0, 0, 0);

        if (RigCurveDataTools::isValidValue(resultValues[i], false))
        {
            result = resultValues[i] * factor * pointNormals[i];
        }

        (*vertices)[i] = cvf::Vec3f(
            displayCoordTransform->transformToDisplayCoord(interpolatedWellPathPoints[i] + pointNormals[i] * offset + result));
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
std::vector<cvf::Vec3d>
    Riv3dWellLogCurveGeometryGenerator::calculatePointNormals(Rim3dWellLogCurve::DrawPlane   drawPlane,
                                                              const std::vector<cvf::Vec3d>& wellPathPoints)
{
    std::vector<cvf::Vec3d> lineSegmentNormals;

    if (wellPathPoints.empty())
    {
        return lineSegmentNormals;
    }

    lineSegmentNormals.reserve(wellPathPoints.size() - 1);
    
    const cvf::Vec3d globalDirection = (wellPathPoints.back() - wellPathPoints.front()).getNormalized();
    const cvf::Vec3d up(0, 0, 1);

    for (size_t i = 0; i < wellPathPoints.size() - 1; i += 2)
    {
        cvf::Vec3d vecAlongPath = (wellPathPoints[i + 1] - wellPathPoints[i]).getNormalized();

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

        cvf::Vec3d normal;

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
            default: break;
        }

        lineSegmentNormals.push_back(normal);
    }

    std::vector<cvf::Vec3d> pointNormals;
    pointNormals.resize(wellPathPoints.size());

    pointNormals[0] = lineSegmentNormals[0];

    for (size_t i = 1; i < pointNormals.size() - 1; i += 2)
    {
        size_t rightSegmentIdx = (i + 1) / 2;
        size_t leftSegmentIdx  = rightSegmentIdx - 1;

        pointNormals[i]     = ((lineSegmentNormals[leftSegmentIdx] + lineSegmentNormals[rightSegmentIdx]) / 2).getNormalized();
        pointNormals[i + 1] = pointNormals[i];
    }

    pointNormals[pointNormals.size() - 1] = lineSegmentNormals.back();

    return pointNormals;
}
