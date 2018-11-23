/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Statoil ASA
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

#include "RivPolylineGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfPrimitiveSetDirect.h"
#include "cvfPrimitiveSetIndexedUInt.h"


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivPolylineGenerator::createLineAlongPolylineDrawable(const std::vector<cvf::Vec3d>& polyLine)
{
    std::vector<std::vector<cvf::Vec3d>> polyLines;
    polyLines.push_back(polyLine);
    return createLineAlongPolylineDrawable(polyLines);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo>
    RivPolylineGenerator::createLineAlongPolylineDrawable(const std::vector<std::vector<cvf::Vec3d>>& polyLines)
{
    std::vector<cvf::uint>  lineIndices;
    std::vector<cvf::Vec3f> vertices;

    for (const std::vector<cvf::Vec3d>& polyLine : polyLines)
    {
        if (polyLine.size() < 2) continue;

        for (size_t i = 0; i < polyLine.size(); ++i)
        {
            vertices.emplace_back(polyLine[i]);
            if (i < polyLine.size() - 1)
            {
                lineIndices.push_back(static_cast<cvf::uint>(i));
                lineIndices.push_back(static_cast<cvf::uint>(i + 1));
            }
        }
    }

    if (vertices.empty()) return nullptr;

    cvf::ref<cvf::Vec3fArray> vx = new cvf::Vec3fArray;
    vx->assign(vertices);
    cvf::ref<cvf::UIntArray> idxes = new cvf::UIntArray;
    idxes->assign(lineIndices);

    cvf::ref<cvf::PrimitiveSetIndexedUInt> prim = new cvf::PrimitiveSetIndexedUInt(cvf::PT_LINES);
    prim->setIndices(idxes.p());

    cvf::ref<cvf::DrawableGeo> polylineGeo = new cvf::DrawableGeo;
    polylineGeo->setVertexArray(vx.p());
    polylineGeo->addPrimitiveSet(prim.p());

    return polylineGeo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivPolylineGenerator::createPointsFromPolylineDrawable(const std::vector<cvf::Vec3d>& polyLine)
{
    std::vector<std::vector<cvf::Vec3d>> polyLines;
    polyLines.push_back(polyLine);
    return createPointsFromPolylineDrawable( polyLines );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo>
    RivPolylineGenerator::createPointsFromPolylineDrawable(const std::vector<std::vector<cvf::Vec3d>>& polyLines)
{
    std::vector<cvf::Vec3f> vertices;

    for (const std::vector<cvf::Vec3d>& polyLine : polyLines)
    {
        for (const auto& pl : polyLine)
        {
            vertices.emplace_back(pl);
        }
    }

    if (vertices.empty()) return nullptr;

    cvf::ref<cvf::PrimitiveSetDirect> primSet = new cvf::PrimitiveSetDirect(cvf::PT_POINTS);
    primSet->setStartIndex(0);
    primSet->setIndexCount(vertices.size());

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;

    cvf::ref<cvf::Vec3fArray> vx = new cvf::Vec3fArray(vertices);
    geo->setVertexArray(vx.p());
    geo->addPrimitiveSet(primSet.p());

    return geo;
}
