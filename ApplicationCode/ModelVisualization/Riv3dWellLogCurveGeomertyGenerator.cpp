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

#include "cvfPrimitiveSetIndexedUInt.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> Riv3dWellLogCurveGeometryGenerator::createDrawable(const std::vector<cvf::Vec3f>& vertices, const std::vector<cvf::uint>& indices) const
{
    cvf::ref<cvf::PrimitiveSetIndexedUInt> indexedUInt = new cvf::PrimitiveSetIndexedUInt(cvf::PrimitiveType::PT_LINES);
    cvf::ref<cvf::UIntArray>               indexArray = new cvf::UIntArray(indices);

    cvf::ref<cvf::DrawableGeo> drawable = new cvf::DrawableGeo();

    indexedUInt->setIndices(indexArray.p());
    drawable->addPrimitiveSet(indexedUInt.p());

    cvf::ref<cvf::Vec3fArray> vertexArray = new cvf::Vec3fArray(vertices);
    drawable->setVertexArray(vertexArray.p());

    return drawable;
}
