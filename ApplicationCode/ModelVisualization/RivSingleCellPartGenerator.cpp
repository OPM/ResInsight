/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RivSingleCellPartGenerator.h"

#include "RigEclipseCaseData.h"
#include "RigFemPartCollection.h"
#include "RigGeoMechCaseData.h"
#include "RigGridBase.h"

#include "RimGeoMechCase.h"

#include "RivFemPartGeometryGenerator.h"
#include "RivPartPriority.h"

#include "cafEffectGenerator.h"
#include "cvfPart.h"
#include "cvfRenderStateDepth.h"
#include "cvfStructGridGeometryGenerator.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivSingleCellPartGenerator::RivSingleCellPartGenerator(RigEclipseCaseData* rigCaseData, size_t gridIndex, size_t cellIndex)
    : m_rigCaseData(rigCaseData),
    m_gridIndex(gridIndex),
    m_cellIndex(cellIndex),
    m_geoMechCase(NULL)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivSingleCellPartGenerator::RivSingleCellPartGenerator(RimGeoMechCase* rimGeoMechCase, size_t gridIndex, size_t cellIndex)
    : m_geoMechCase(rimGeoMechCase),
    m_gridIndex(gridIndex),
    m_cellIndex(cellIndex),
    m_rigCaseData(NULL)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Part> RivSingleCellPartGenerator::createPart(const cvf::Color3f color)
{
    cvf::ref<cvf::Part> part = new cvf::Part;
    part->setName(cvf::String("Hightlight part for cell index ") + cvf::String((cvf::int64)m_cellIndex));
    part->setDrawable(createMeshDrawable().p());

    cvf::ref<cvf::Effect> eff;
    caf::MeshEffectGenerator effGen(color);
    eff = effGen.generateUnCachedEffect();

    cvf::ref<cvf::RenderStateDepth> depth = new cvf::RenderStateDepth;
    depth->enableDepthTest(false);
    eff->setRenderState(depth.p());

    part->setEffect(eff.p());

    part->setPriority(RivPartPriority::PartType::Highlight);

    return part;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivSingleCellPartGenerator::createMeshDrawable()
{
    if (m_rigCaseData && m_cellIndex != cvf::UNDEFINED_SIZE_T)
    {
        return cvf::StructGridGeometryGenerator::createMeshDrawableFromSingleCell(m_rigCaseData->grid(m_gridIndex), m_cellIndex);
    }
    else if (m_geoMechCase && m_cellIndex != cvf::UNDEFINED_SIZE_T)
    {
        CVF_ASSERT(m_geoMechCase->geoMechData());
        CVF_ASSERT(m_geoMechCase->geoMechData()->femParts()->partCount() > static_cast<int>(m_gridIndex));

        RigFemPart* femPart = m_geoMechCase->geoMechData()->femParts()->part(m_gridIndex);
        CVF_ASSERT(femPart);

        return RivFemPartGeometryGenerator::createMeshDrawableFromSingleElement(femPart, m_cellIndex);
    }

    return NULL;
}
