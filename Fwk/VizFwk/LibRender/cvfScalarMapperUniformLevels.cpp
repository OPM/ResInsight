//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#include "cvfBase.h"
#include "cvfMath.h"
#include "cvfScalarMapperUniformLevels.h"
#include "cvfOverlayColorLegend.h"
#include "cvfTextureImage.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::ScalarMapperUniformLevels
/// \ingroup Render
///
/// Maps scalar values to texture coordinates/colors using levels of uniform size. 
/// Configured by specifying a number of level colors and a min/max range. 
//==================================================================================================
ScalarMapperUniformLevels::ScalarMapperUniformLevels()
:   m_rangeMin(0),
    m_rangeMax(0),
    m_textureSize(64),
    m_maxTexCoord(1.0)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ScalarMapperUniformLevels::setRange(double min, double max)
{
    m_rangeMin = min;
    m_rangeMax = max;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double ScalarMapperUniformLevels::rangeMin() const
{
    return m_rangeMin;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double ScalarMapperUniformLevels::rangeMax() const
{
    return m_rangeMax;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ScalarMapperUniformLevels::setColors(const Color3ubArray& colorArray)
{
    CVF_ASSERT(colorArray.size() > 0);
    m_colors = colorArray;

    recomputeMaxTexCoord();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ScalarMapperUniformLevels::setColors(ColorTable colorTable, uint levelCount)
{
    // The normal color table has 13 hand coded levels, so handle these level counts separately
    if (colorTable == NORMAL && levelCount <= 13)
    {
        m_colors = *normalColorTableArray(levelCount);
    }
    else
    {
        ref<Color3ubArray> baseColors = colorTableArray(colorTable);
        if (baseColors->size() == levelCount)
        {
            m_colors = *baseColors;
        }
        else
        {
            m_colors = *interpolateColorArray(*baseColors, levelCount);
        }
    }

    CVF_ASSERT(m_colors.size() == levelCount);

    recomputeMaxTexCoord();
}


//--------------------------------------------------------------------------------------------------
/// Set the size of the texture produces by updateTexture()
/// 
/// For compatibility with OpenGL versions older than 2.0, the specified size should be a power of 2
/// Note that changing this size affects the texture coordinate generation.
///
/// \warning The texture size must be at least the the same as the number of colors specified.
///          If this is not the case, updateTexture() will fail.
//--------------------------------------------------------------------------------------------------
void ScalarMapperUniformLevels::setTextureSize(uint textureSize)
{
    CVF_ASSERT(m_textureSize > 0);
    m_textureSize = textureSize;

    recomputeMaxTexCoord();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
uint ScalarMapperUniformLevels::textureSize() const
{
    return m_textureSize;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ScalarMapperUniformLevels::recomputeMaxTexCoord()
{
    const uint numColors = static_cast<uint>(m_colors.size());
    if (numColors == 0)
    {
        m_maxTexCoord = 1.0;
        return;
    }

    const uint numPixelsPerColor = m_textureSize/numColors;
    if (numPixelsPerColor == 0)
    {
        m_maxTexCoord = 1.0;
        return;
    }

    uint texturePixelsInUse = numColors*numPixelsPerColor;
    CVF_ASSERT(texturePixelsInUse <= m_textureSize);

    m_maxTexCoord = static_cast<double>(texturePixelsInUse)/static_cast<double>(m_textureSize);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Vec2f ScalarMapperUniformLevels::mapToTextureCoord(double scalarValue) const
{
    const double range = m_rangeMax - m_rangeMin;
    if (range >= 0)
    {
        if (scalarValue > m_rangeMin && scalarValue < m_rangeMax)
        {
            // Truly inside the range, implies a range > 0
            CVF_ASSERT(range > 0);
            double s = static_cast<float>(((scalarValue - m_rangeMin)/range)*m_maxTexCoord);

            // Clamp to the currently legal texture coord range
            // Might need to add code to correct for float precision, but that is probably not the main enemy.
            // Our real problem is the fact that in most cases the texture coords get treated with even less precision
            // on the graphics hardware. What we would really like is to guess at the HW precision and then correct for that.
            // Currently the workaround is done in updateTexture() which pads the upper end of the texture when we're not filling
            // all the texture pixels.
            s = Math::clamp(s, 0.0, m_maxTexCoord);

            return Vec2f(static_cast<float>(s), 0.5f);    
        }
        else if (scalarValue <= m_rangeMin)
        {
            return Vec2f(0.0f, 0.5f);
        }
        else if (scalarValue >= m_rangeMax)
        {
            return Vec2f(static_cast<float>(m_maxTexCoord), 0.5f);
        }
        else 
        {
            // If we get here we have som NaN, Use lowest legal texture coord
            return Vec2f(0.0f, 0.0f);
        }
    }
    else
    {
        // Range is invalid (min > max)
        return Vec2f(0.0f, 0.5f);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color3ub ScalarMapperUniformLevels::mapToColor(double scalarValue) const
{
    uint colorCount = static_cast<uint>(m_colors.size());
    CVF_ASSERT(colorCount > 0);
    if (colorCount < 1)
    {
        return Color3::BLACK;
    }

    const double range = m_rangeMax - m_rangeMin;
    if (range >= 0)
    {
        if (scalarValue > m_rangeMin && scalarValue < m_rangeMax)
        {
            // Truly inside the range, implies a range > 0
            CVF_ASSERT(range > 0);
            const double levelRange = range/colorCount;
            const double dblIndex = (scalarValue - m_rangeMin)/levelRange;
            
            uint index  = 0;
            if (dblIndex > 0 && dblIndex < colorCount)
            {
                index = static_cast<uint>(dblIndex);
            }
            else if (dblIndex >= colorCount)
            {
                index = colorCount - 1;
            }

            CVF_ASSERT(index < colorCount);
            return m_colors[index];
        }
        else if (scalarValue <= m_rangeMin)
        {
            return m_colors[0];
        }
        else if (scalarValue >= m_rangeMax)
        {
            return m_colors[colorCount - 1];
        }
        else 
        {
            // If we get here we have some NaN 
            return Color3::BLACK;        
        }
    }
    else
    {
        // Range is invalid
        return m_colors[0];
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ScalarMapperUniformLevels::updateColorLegend(OverlayColorLegend* legend) const
{
    CVF_ASSERT(legend);

    size_t numTicks = m_colors.size() + 1;
    if (numTicks < 2)
    {
        return false;
    }

    DoubleArray ticks;
    ticks.reserve(numTicks);

    double delta = (m_rangeMax - m_rangeMin)/static_cast<double>(numTicks - 1);

    size_t i;
    for (i = 0; i < numTicks - 1; i++)
    {
        double tickVal = m_rangeMin + static_cast<double>(i)*delta;
        ticks.add(tickVal);
    }

    ticks.add(m_rangeMax);
    Color3ubArray colorArr(m_colors);

    legend->configureLevels(colorArr, ticks);

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ScalarMapperUniformLevels::updateTexture(TextureImage* image) const
{
    CVF_ASSERT(image);

    const uint numColors = static_cast<uint>(m_colors.size());
    if (numColors < 1)
    {
        return false;
    }

    CVF_ASSERT(m_textureSize >= numColors);
    if (m_textureSize < numColors)
    {
        return false;
    }

    image->allocate(m_textureSize, 1);

    // For now fill with white so we can see any errors more easily
    image->fill(Color4ub(Color3::WHITE));

    const uint numPixelsPerColor = m_textureSize/numColors;
    CVF_ASSERT(numPixelsPerColor >= 1);

    uint ic;
    for (ic = 0; ic < numColors; ic++)
    {
        const Color4ub clr(m_colors[ic], 255);

        uint ip;
        for (ip = 0; ip < numPixelsPerColor; ip++)
        {
            image->setPixel(ic*numPixelsPerColor + ip, 0, clr);
        }
    }

    // In cases where we're not using the entire texture we might get into problems with texture coordinate precision on the graphics hardware.
    // Therefore we set one extra pixel with the 'highest' color in the color table
    if (numColors*numPixelsPerColor < m_textureSize)
    {
        const Color4ub topClr(m_colors[numColors - 1], 255);
        image->setPixel(numColors*numPixelsPerColor, 0, topClr);
    }

    return true;
}

} // namespace cvf

