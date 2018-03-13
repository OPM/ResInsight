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

    std::vector<cvf::Vec3f> vertices;
    vertices.reserve(wellPathPoints.size() * 2);

    std::vector<cvf::Vec3d> curveNormals;
    curveNormals.reserve(wellPathPoints.size());

    for (size_t i = 0; i < wellPathPoints.size() - 1; i++)
    {
        cvf::Vec3d z = zForDrawPlane(rim3dWellLogCurve->drawPlane());
        cvf::Vec3d y = normalBetweenPoints(wellPathPoints[i], wellPathPoints[i + 1], z);

        curveNormals.push_back(y);
    }

    std::vector<cvf::uint> indices;
    vertices.reserve(wellPathPoints.size() * 2);

    cvf::uint counter = 0;

    for (size_t i = 0; i < curveNormals.size(); i++)
    {
        vertices.push_back(cvf::Vec3f(displayCoordTransform->transformToDisplayCoord(wellPathPoints[i])));
        vertices.push_back(cvf::Vec3f(displayCoordTransform->transformToDisplayCoord(wellPathPoints[i] + curveNormals[i] * 100)));

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
    std::vector<double> resultValues;
    std::vector<double> mds;
    rim3dWellLogCurve->resultValuesAndMds(&resultValues, &mds);

    CVF_ASSERT(resultValues.size() == mds.size());

    std::vector<cvf::Vec3d> wellPathPoints;
    wellPathPoints.reserve(mds.size());

    for (double md : mds)
    {
        wellPathPoints.push_back(m_wellPathGeometry->interpolatedPointAlongWellPath(md));
    }

    vertices->resize(wellPathPoints.size());

    std::vector<cvf::Vec3d> curveNormals;
    curveNormals.reserve(wellPathPoints.size());

    for (size_t i = 0; i < wellPathPoints.size() - 1; i += 2)
    {
        cvf::Vec3d z = zForDrawPlane(rim3dWellLogCurve->drawPlane());

        cvf::Vec3d y = normalBetweenPoints(wellPathPoints[i], wellPathPoints[i + 1], z);

        curveNormals.push_back(y);
        curveNormals.push_back(y);
    }

    double maxResult = -HUGE_VAL;
    double minResult = HUGE_VAL;

    for (double result : resultValues)
    {
        if (!RigCurveDataTools::isValidValue(result, false)) continue;

        maxResult = std::max(result, maxResult);
        minResult = std::min(result, minResult);
    }

    double range  = maxResult - minResult;
    double factor = 60.0 / range;

    for (size_t i = 0; i < curveNormals.size(); i++)
    {
        cvf::Vec3d result(0, 0, 0);

        if (RigCurveDataTools::isValidValue(resultValues[i], false))
        {
            result = resultValues[i] * factor * curveNormals[i];
        }

        (*vertices)[i] =
            cvf::Vec3f(displayCoordTransform->transformToDisplayCoord(wellPathPoints[i] + curveNormals[i] * 30 + result));
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
cvf::Vec3d Riv3dWellLogCurveGeometryGenerator::normalBetweenPoints(const cvf::Vec3d& pt1,
                                                                   const cvf::Vec3d& pt2,
                                                                   const cvf::Vec3d& z) const
{
    cvf::Vec3d x = (pt2 - pt1).getNormalized();

    return (z ^ x).getNormalized();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d Riv3dWellLogCurveGeometryGenerator::zForDrawPlane(const Rim3dWellLogCurve::DrawPlane& drawPlane) const
{
    if (drawPlane == Rim3dWellLogCurve::HORIZONTAL_LEFT)
    {
        return cvf::Vec3d(0, 0, -1);
    }
    else if (drawPlane == Rim3dWellLogCurve::HORIZONTAL_RIGHT)
    {
        return cvf::Vec3d(0, 0, 1);
    }
    else if (drawPlane == Rim3dWellLogCurve::VERTICAL_ABOVE)
    {
        return cvf::Vec3d(0, -1, 0);
    }
    else if (drawPlane == Rim3dWellLogCurve::VERTICAL_BELOW)
    {
        return cvf::Vec3d(0, 1, 0);
    }
    else
    {
        // Default: Horizontal left
        return cvf::Vec3d(0, 0, -1);
    }
}
