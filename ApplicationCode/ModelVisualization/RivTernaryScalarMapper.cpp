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

#include "RivTernaryScalarMapper.h"
#include "cvfTextureImage.h"



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivTernaryScalarMapper::RivTernaryScalarMapper(const cvf::Color3f& undefScalarColor)
:	m_undefScalarColor(undefScalarColor),
	m_textureSize(128, 256)
{
	m_opacityLevel = 1.0;

	setTernaryRanges(0.0, 1.0, 0.0, 1.0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivTernaryScalarMapper::setOpacityLevel(float opacityLevel)
{
	m_opacityLevel = opacityLevel;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec2f RivTernaryScalarMapper::mapToTextureCoord(double soil, double sgas, bool isTransparent) const
{
	double soilNormalized = (soil - m_rangeMinSoil) * m_soilFactor;
	soilNormalized = cvf::Math::clamp(soilNormalized, 0.0, 1.0);

	double sgasNormalized = (sgas - m_rangeMinSgas) * m_sgasFactor;
	sgasNormalized = cvf::Math::clamp(sgasNormalized, 0.0, 1.0 - soilNormalized);
	sgasNormalized /= 2.0;

	if (isTransparent)
	{
		sgasNormalized += 0.5;
	}

	cvf::Vec2f texCoord(static_cast<float>(soilNormalized), static_cast<float>(sgasNormalized));
	return texCoord;
}

//--------------------------------------------------------------------------------------------------
/// F * 
///   * *
///   *   *  
///   *     *				Texture in this region is assigned the given opacity level
///   *       *  
/// D ***********  E    
/// C * SGAS
///   * *
///   *   *					Texture in this region is opaque
///   *     *  
///   *       *  
/// A *********** B
/// SWAT          SOIL
//--------------------------------------------------------------------------------------------------
bool RivTernaryScalarMapper::updateTexture(cvf::TextureImage* image)
{
	CVF_ASSERT(image);
	image->allocate(m_textureSize.x(), m_textureSize.y());

	// For now fill with white so we can see any errors more easily
	image->fill(cvf::Color4ub(cvf::Color3::WHITE));

	
	
	cvf::uint halfTextureHeight = m_textureSize.y() / 2;
	
	// Create texture 

	float xStride = static_cast<float>(1.0f / m_textureSize.x());
	float yStride = static_cast<float>(1.0f / halfTextureHeight);

	float sgas_red = 0.0f;
	for (cvf::uint yPos = 0; yPos < halfTextureHeight; yPos++)
	{
		float soil_green = 0.0f;
		for (cvf::uint xPos = 0; xPos < m_textureSize.x() - yPos; xPos++)
		{
			float swat_blue = 1.0f - sgas_red - soil_green;

			cvf::Color3f floatCol(sgas_red, soil_green, swat_blue);

			cvf::ubyte rByteCol = floatCol.rByte();
			cvf::ubyte gByteCol = floatCol.gByte();
			cvf::ubyte bByteCol = floatCol.bByte();

			const cvf::Color4ub clr(rByteCol, gByteCol, bByteCol, 255);
			image->setPixel(xPos, yPos, clr);

			// Set opacity
			const cvf::Color4ub clrOpacity(rByteCol, gByteCol, bByteCol, static_cast<cvf::ubyte>(255 * m_opacityLevel));
			image->setPixel(xPos, yPos + halfTextureHeight, clrOpacity);

			soil_green += xStride;
		}
		sgas_red += yStride;
	}

	return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivTernaryScalarMapper::setTernaryRanges(double soilLower, double soilUpper, double sgasLower, double sgasUpper)
{
	m_rangeMinSoil = soilLower;
	m_rangeMaxSoil = soilUpper;
	m_soilFactor = 1.0 / (soilUpper - soilLower);

	m_rangeMinSgas = sgasLower;
	m_rangeMaxSgas = sgasUpper;
	m_sgasFactor = 1.0 / (sgasUpper - sgasLower);
}

