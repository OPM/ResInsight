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

#include "RivScalarMapperUtils.h"

#include "RiaColorTables.h"

#include "RimCellEdgeColors.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimRegularLegendConfig.h"
#include "RimTernaryLegendConfig.h"

#include "RivCellEdgeEffectGenerator.h"
#include "RivCellEdgeGeometryUtils.h"
#include "RivTernaryScalarMapper.h"
#include "RivTernaryScalarMapperEffectGenerator.h"

#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfPart.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivScalarMapperUtils::applyTextureResultsToPart(cvf::Part* part, cvf::Vec2fArray* textureCoords, const cvf::ScalarMapper* mapper, float opacityLevel, caf::FaceCulling faceCulling, bool disableLighting)
{
    CVF_ASSERT(part && textureCoords && mapper);

    cvf::DrawableGeo* dg = dynamic_cast<cvf::DrawableGeo*>(part->drawable());
    if (dg) dg->setTextureCoordArray(textureCoords);

    cvf::ref<cvf::Effect> scalarEffect = RivScalarMapperUtils::createScalarMapperEffect(mapper, opacityLevel, faceCulling, disableLighting);
    part->setEffect(scalarEffect.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivScalarMapperUtils::applyTernaryTextureResultsToPart(cvf::Part* part, cvf::Vec2fArray* textureCoords, const RivTernaryScalarMapper* mapper, float opacityLevel, caf::FaceCulling faceCulling, bool disableLighting)
{
    CVF_ASSERT(part && textureCoords && mapper);

    cvf::DrawableGeo* dg = dynamic_cast<cvf::DrawableGeo*>(part->drawable());
    if (dg) dg->setTextureCoordArray(textureCoords);

    cvf::ref<cvf::Effect> scalarEffect = RivScalarMapperUtils::createTernaryScalarMapperEffect(mapper, opacityLevel, faceCulling, disableLighting);
    part->setEffect(scalarEffect.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Effect> RivScalarMapperUtils::createCellEdgeEffect(cvf::DrawableGeo* dg,
    const cvf::StructGridQuadToCellFaceMapper* quadToCellFaceMapper,
    size_t gridIndex,
    size_t timeStepIndex,
    RimEclipseCellColors* cellResultColors,
    RimCellEdgeColors* cellEdgeResultColors,
    float opacityLevel,
    cvf::Color3f defaultColor,
    caf::FaceCulling faceCulling,
    bool disableLighting)
{
    CellEdgeEffectGenerator cellFaceEffectGen(cellEdgeResultColors->legendConfig()->scalarMapper());

    if (cellResultColors->isTernarySaturationSelected())
    {
        RivCellEdgeGeometryUtils::addTernaryCellEdgeResultsToDrawableGeo(timeStepIndex, cellResultColors, cellEdgeResultColors,
            quadToCellFaceMapper, dg, gridIndex, opacityLevel);

        const RivTernaryScalarMapper* ternaryCellScalarMapper = cellResultColors->ternaryLegendConfig()->scalarMapper();
        cellFaceEffectGen.setTernaryScalarMapper(ternaryCellScalarMapper);
    }
    else
    {
        bool useDefaultValueForHugeVals = false;
        if (!cellResultColors->hasResult())
        {
            useDefaultValueForHugeVals = true;
        }

        RivCellEdgeGeometryUtils::addCellEdgeResultsToDrawableGeo(timeStepIndex, cellResultColors, cellEdgeResultColors,
            quadToCellFaceMapper, dg, gridIndex, useDefaultValueForHugeVals, opacityLevel);

        if (cellResultColors->hasResult())
        {
            // If no scalar mapper is set for the effect, a default color is used to fill the texture
            // This is what we want when the fault colors should be visible in combination with cell edge
            cvf::ScalarMapper* cellScalarMapper = cellResultColors->legendConfig()->scalarMapper();
            cellFaceEffectGen.setScalarMapper(cellScalarMapper);
        }
    }

    cellFaceEffectGen.setOpacityLevel(opacityLevel);
    cellFaceEffectGen.setDefaultCellColor(defaultColor);
    cellFaceEffectGen.setFaceCulling(faceCulling);
    cellFaceEffectGen.disableLighting(disableLighting);

    cvf::ref<cvf::Effect> eff = cellFaceEffectGen.generateCachedEffect();
    return eff;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Effect> RivScalarMapperUtils::createScalarMapperEffect(const cvf::ScalarMapper* mapper, float opacityLevel, caf::FaceCulling faceCulling, bool disableLighting)
{
    CVF_ASSERT(mapper);

    caf::PolygonOffset polygonOffset = caf::PO_1;
    caf::ScalarMapperEffectGenerator scalarEffgen(mapper, polygonOffset);
    scalarEffgen.setOpacityLevel(opacityLevel);
    scalarEffgen.setFaceCulling(faceCulling);
    scalarEffgen.setUndefinedColor(RiaColorTables::undefinedCellColor());
    scalarEffgen.disableLighting(disableLighting);

    cvf::ref<cvf::Effect> scalarEffect = scalarEffgen.generateCachedEffect();

    return scalarEffect;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Effect> RivScalarMapperUtils::createTernaryScalarMapperEffect(const RivTernaryScalarMapper* mapper, float opacityLevel, caf::FaceCulling faceCulling, bool disableLighting)
{
    CVF_ASSERT(mapper);

    caf::PolygonOffset polygonOffset = caf::PO_1;
    RivTernaryScalarMapperEffectGenerator scalarEffgen(mapper, polygonOffset);
    scalarEffgen.setOpacityLevel(opacityLevel);
    scalarEffgen.setFaceCulling(faceCulling);
    scalarEffgen.setUndefinedColor(RiaColorTables::undefinedCellColor());
    scalarEffgen.disableLighting(disableLighting);

    cvf::ref<cvf::Effect> scalarEffect = scalarEffgen.generateCachedEffect();

    return scalarEffect;
}

