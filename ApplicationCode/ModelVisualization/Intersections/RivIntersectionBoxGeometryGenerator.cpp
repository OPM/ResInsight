/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "RivIntersectionBoxGeometryGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfPrimitiveSetDirect.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivIntersectionBoxGeometryGenerator::RivIntersectionBoxGeometryGenerator(const RimIntersectionBox* intersectionBox, const RivIntersectionHexGridInterface* grid)
    : m_crossSection(intersectionBox),
    m_hexGrid(grid)
{
    m_triangleVxes = new cvf::Vec3fArray;
    m_cellBorderLineVxes = new cvf::Vec3fArray;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivIntersectionBoxGeometryGenerator::~RivIntersectionBoxGeometryGenerator()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RivIntersectionBoxGeometryGenerator::isAnyGeometryPresent() const
{
    if (m_triangleVxes->size() == 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivIntersectionBoxGeometryGenerator::generateSurface()
{
    calculateArrays();

    CVF_ASSERT(m_triangleVxes.notNull());

    if (m_triangleVxes->size() == 0) return NULL;

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->setFromTriangleVertexArray(m_triangleVxes.p());

    return geo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivIntersectionBoxGeometryGenerator::createMeshDrawable()
{
    if (!(m_cellBorderLineVxes.notNull() && m_cellBorderLineVxes->size() != 0)) return NULL;

    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;
    geo->setVertexArray(m_cellBorderLineVxes.p());


    cvf::ref<cvf::PrimitiveSetDirect> prim = new cvf::PrimitiveSetDirect(cvf::PT_LINES);
    prim->setIndexCount(m_cellBorderLineVxes->size());

    geo->addPrimitiveSet(prim.p());
    return geo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RivIntersectionBoxGeometryGenerator::triangleToCellIndex() const
{
    CVF_ASSERT(m_triangleVxes->size());
    return m_triangleToCellIdxMap;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<RivIntersectionVertexWeights>& RivIntersectionBoxGeometryGenerator::triangleVxToCellCornerInterpolationWeights() const
{
    CVF_ASSERT(m_triangleVxes->size());
    return m_triVxToCellCornerWeights;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivIntersectionBoxGeometryGenerator::calculateArrays()
{

}
