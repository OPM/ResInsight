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

#include "cafEffectGenerator.h"

class RivTernaryScalarMapper;

namespace cvf
{
    class TextureImage;
}

//==================================================================================================
//
// ScalarMapperEffectGenerator
//
//==================================================================================================
class RivTernaryScalarMapperEffectGenerator : public caf::EffectGenerator
{
public:
    RivTernaryScalarMapperEffectGenerator(const RivTernaryScalarMapper* scalarMapper, caf::PolygonOffset polygonOffset);

    void                            setOpacityLevel(float opacity)          { m_opacityLevel = cvf::Math::clamp(opacity, 0.0f, 1.0f); }
    void                            setUndefinedColor(cvf::Color3f color)   { m_undefinedColor = color; }
    void                            setFaceCulling(caf::FaceCulling faceCulling) { m_faceCulling = faceCulling; }
    void                            enableDepthWrite(bool enableWrite)      { m_enableDepthWrite = enableWrite; }
    void                            disableLighting(bool disable)           { m_disableLighting = disable; }


public:
    static bool                     isImagesEqual(const cvf::TextureImage* texImg1, const cvf::TextureImage* texImg2);

protected:
    bool                    isEqual(const caf::EffectGenerator* other) const override;
    caf::EffectGenerator*    copy() const override;

    void                    updateForShaderBasedRendering(cvf::Effect* effect) const override;
    void                    updateForFixedFunctionRendering(cvf::Effect* effect) const override;

private:
    void                            updateCommonEffect(cvf::Effect* effect) const;

private:
    cvf::cref<RivTernaryScalarMapper>    m_scalarMapper;
    mutable cvf::ref<cvf::TextureImage>    m_textureImage;
    caf::PolygonOffset                  m_polygonOffset;
    float                                m_opacityLevel;
    cvf::Color3f                        m_undefinedColor;
    caf::FaceCulling                    m_faceCulling;
    bool                                m_enableDepthWrite;
    bool                                m_disableLighting;
};

