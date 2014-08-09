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
	RivTernaryScalarMapper(const cvf::Color3f& undefScalarColor, float opacityLevel);

	cvf::Vec2f	mapToTextureCoord(double soil, double swat, bool isTransparent);
	bool		updateTexture(cvf::TextureImage* image);

private:
	cvf::Color3f	m_undefScalarColor;
	float			m_opacityLevel;
	cvf::Vec2ui		m_textureSize;
};

