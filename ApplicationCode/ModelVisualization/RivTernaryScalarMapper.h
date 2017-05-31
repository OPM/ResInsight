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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfVector2.h"
#include "cvfColor3.h"

namespace cvf
{
    class TextureImage;
}

//==================================================================================================
///
//==================================================================================================
class RivTernaryScalarMapper : public cvf::Object
{
public:
    explicit RivTernaryScalarMapper(const cvf::Color3f& undefScalarColor);

    void setTernaryRanges(double soilLower, double soilUpper, double sgasLower, double sgasUpper);

    cvf::Vec2f    mapToTextureCoord(double soil, double sgas, bool isTransparent) const;
    bool        updateTexture(cvf::TextureImage* image, float opacityLevel) const;

private:
    cvf::Color3f    m_undefScalarColor;
    cvf::Vec2ui        m_textureSize;

    double m_rangeMaxSoil;
    double m_rangeMinSoil;
    double m_soilFactor;

    double m_rangeMaxSgas;
    double m_rangeMinSgas;
    double m_sgasFactor;
};

