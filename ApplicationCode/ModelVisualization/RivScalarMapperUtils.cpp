/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA, Ceetron Solutions AS
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

#include "RivTernaryScalarMapperEffectGenerator.h"
#include "RivTernaryScalarMapper.h"

#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfPart.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivScalarMapperUtils::applyTextureResultsToPart(cvf::Part* part, cvf::Vec2fArray* textureCoords, const cvf::ScalarMapper* mapper, float opacityLevel)
{
	CVF_ASSERT(part && textureCoords && mapper);

	cvf::DrawableGeo* dg = dynamic_cast<cvf::DrawableGeo*>(part->drawable());
	if (dg) dg->setTextureCoordArray(textureCoords);

	cvf::ref<cvf::Effect> scalarEffect = RivScalarMapperUtils::createScalarMapperEffect(mapper, opacityLevel);
	part->setEffect(scalarEffect.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivScalarMapperUtils::applyTernaryTextureResultsToPart(cvf::Part* part, cvf::Vec2fArray* textureCoords, const RivTernaryScalarMapper* mapper, float opacityLevel)
{
	CVF_ASSERT(part && textureCoords && mapper);

	cvf::DrawableGeo* dg = dynamic_cast<cvf::DrawableGeo*>(part->drawable());
	if (dg) dg->setTextureCoordArray(textureCoords);

	cvf::ref<cvf::Effect> scalarEffect = RivScalarMapperUtils::createTernaryScalarMapperEffect(mapper, opacityLevel);
	part->setEffect(scalarEffect.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Effect> RivScalarMapperUtils::createScalarMapperEffect(const cvf::ScalarMapper* mapper, float opacityLevel)
{
	CVF_ASSERT(mapper);

	caf::PolygonOffset polygonOffset = caf::PO_1;
	caf::ScalarMapperEffectGenerator scalarEffgen(mapper, polygonOffset);
	scalarEffgen.setOpacityLevel(opacityLevel);
	cvf::ref<cvf::Effect> scalarEffect = scalarEffgen.generateEffect();

	return scalarEffect;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Effect> RivScalarMapperUtils::createTernaryScalarMapperEffect(const RivTernaryScalarMapper* mapper, float opacityLevel)
{
	CVF_ASSERT(mapper);

	caf::PolygonOffset polygonOffset = caf::PO_1;
	RivTernaryScalarMapperEffectGenerator scalarEffgen(mapper, polygonOffset);
	scalarEffgen.setOpacityLevel(opacityLevel);
	cvf::ref<cvf::Effect> scalarEffect = scalarEffgen.generateEffect();

	return scalarEffect;
}
