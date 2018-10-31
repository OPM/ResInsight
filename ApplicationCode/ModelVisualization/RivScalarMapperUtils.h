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

#pragma once

#include "RiaColorTables.h"

#include "cafEffectGenerator.h"

#include "cvfBase.h"
#include "cvfArray.h"

namespace cvf
{
    class ScalarMapper;
    class Part;
    class Effect;
    class StructGridQuadToCellFaceMapper;
    class DrawableGeo;
}

class RivTernaryScalarMapper;
class RimEclipseCellColors;
class RimCellEdgeColors;

//==================================================================================================
///
//==================================================================================================
class RivScalarMapperUtils
{
public:
    static void applyTextureResultsToPart(cvf::Part* part, cvf::Vec2fArray* textureCoords, const cvf::ScalarMapper* mapper, float opacityLevel, caf::FaceCulling faceCulling, bool disableLighting, const cvf::Color3f& undefColor = cvf::Color3f(RiaColorTables::undefinedCellColor()));
    static void applyTernaryTextureResultsToPart(cvf::Part* part, cvf::Vec2fArray* textureCoords, const RivTernaryScalarMapper* mapper, float opacityLevel, caf::FaceCulling faceCulling, bool disableLighting);

    static cvf::ref<cvf::Effect> createCellEdgeEffect(cvf::DrawableGeo* dg,
        const cvf::StructGridQuadToCellFaceMapper* quadToCellFaceMapper,
        size_t gridIndex,
        size_t timeStepIndex,
        RimEclipseCellColors* cellResultColors,
        RimCellEdgeColors* cellEdgeResultColors,
        float opacityLevel,
        cvf::Color3f defaultColor, 
        caf::FaceCulling faceCulling,
        bool disableLighting);

private:
    static cvf::ref<cvf::Effect> createScalarMapperEffect(const cvf::ScalarMapper* mapper, float opacityLevel, caf::FaceCulling faceCulling, bool disableLighting, const cvf::Color3f& undefColor = cvf::Color3f(RiaColorTables::undefinedCellColor()));
    static cvf::ref<cvf::Effect> createTernaryScalarMapperEffect(const RivTernaryScalarMapper* mapper, float opacityLevel, caf::FaceCulling faceCulling, bool disableLighting);
};

