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


#include "cvfScalarMapperRangeBased.h"
#include "cvfMath.h"
#include "cvfTextureImage.h"
#include <cmath>

namespace cvf {
//==================================================================================================
///
/// \class cvf::ScalarMapperRangeBased
/// \ingroup Render
///
/// This is a base class for ScalarMapper's using range and levels, and does most of the job 
/// apart from the mapping itself
//==================================================================================================

ScalarMapperRangeBased::ScalarMapperRangeBased()
:   m_rangeMin(cvf::UNDEFINED_DOUBLE),
    m_rangeMax(cvf::UNDEFINED_DOUBLE),
    m_decadeLevelCount(1),
    m_levelCount(8),
    m_adjustLevels(true),
    m_textureSize(2048)  // Large enough, I guess and a power of two
{
    m_interpolatedUserGradientColors.resize(m_textureSize);
    m_interpolatedUserGradientColors.setAll(Color3ub::WHITE);
    setColors(*normalColorTableArray(13));

}


//--------------------------------------------------------------------------------------------------
/// Sets the max and min level of the legend. If the levels previously has been set with setLevelsFromValues() 
/// only the values between the new max min range becomes visible.
//--------------------------------------------------------------------------------------------------
void ScalarMapperRangeBased::setRange(double min, double max)
{
    m_rangeMin = min;
    m_rangeMax = max;
    updateSortedLevels();
    rangeUpdated();
}


//--------------------------------------------------------------------------------------------------
/// Sets the colors that will be used in the legend. Will be interpolated when needed.
//--------------------------------------------------------------------------------------------------
void ScalarMapperRangeBased::setColors(const Color3ubArray& colorArray)
{
    m_interpolatedUserGradientColors = *interpolateColorArray(colorArray, m_textureSize);
}

//--------------------------------------------------------------------------------------------------
/// Sets the colors from a predefined color set that will be used in the legend. 
///  Will be interpolated when needed.
//--------------------------------------------------------------------------------------------------
void ScalarMapperRangeBased::setColors(ColorTable colorTable)
{
    ref<Color3ubArray> baseColors = colorTableArray(colorTable);
    setColors(*baseColors);
}

//--------------------------------------------------------------------------------------------------
/// Sets the number of ranges, creating (levelCount + 1) tickmarks (including max and min) 
//--------------------------------------------------------------------------------------------------
void ScalarMapperRangeBased::setLevelCount(size_t levelCount, bool adjustLevels)
{
    m_userDefinedLevelValues.clear();

    m_levelCount = levelCount;
    m_adjustLevels = adjustLevels;

    updateSortedLevels();
}

//--------------------------------------------------------------------------------------------------
/// This method sets all the levels to the user defined domain values, 
/// overriding any previous max and min range settings.
//--------------------------------------------------------------------------------------------------
void ScalarMapperRangeBased::setLevelsFromValues(const std::set<double>& levelValues)
{
    CVF_ASSERT(!levelValues.empty());

    m_userDefinedLevelValues = levelValues;
    m_rangeMax = (*levelValues.rbegin());
    m_rangeMin = (*levelValues.begin());
    updateSortedLevels();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Vec2f ScalarMapperRangeBased::mapToTextureCoord(double scalarValue) const
{
    return Vec2f(static_cast<float>(normalizedValue(scalarValue)), 0.5f);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color3ub ScalarMapperRangeBased::mapToColor(double scalarValue) const
{
    return colorFromUserColorGradient(normalizedValue(scalarValue));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ScalarMapperRangeBased::updateSortedLevels()
{
    std::vector<double> levels;
    majorTickValues(&levels);
    std::set<double>::iterator it;
    m_sortedLevels.clear();
    m_sortedLevels.insert(levels.begin(), levels.end());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ScalarMapperRangeBased::updateTexture(TextureImage* image) const
{
    CVF_ASSERT(image);

    image->allocate(m_textureSize, 1);

    // For now fill with white so we can see any errors more easily
    image->fill(Color4ub(Color3::WHITE));

    uint ic;
    for (ic = 0; ic < m_textureSize; ic++)
    {
        const Color4ub clr(mapToColor(domainValue(((double)ic)/(m_textureSize-1))), 255); 
        image->setPixel(ic, 0, clr);
    }
 
    return true;
}

// Then calculate a stepsize that is humanly understandable
// basically rounded to whole or half of the decade in question
// decadeParts - The number of steps wanted within a decade
// decadeValue - The value used to describe the current decade to round off within
static double adjust(double domainValue, double decadeValue, unsigned int decadeParts = 2)
{
    if (decadeValue == 0) return domainValue; // Conceptually correct

    //double sign = domainValue >= 0 ? 1.0 : -1.0;

    // Calculate the decade
    decadeValue = cvf::Math::abs(decadeValue);
    double logDecValue = log10(decadeValue );
    logDecValue = cvf::Math::floor(logDecValue);
    double decade = pow(10.0, logDecValue);

    double firstDecadeDomVal = decadeParts*domainValue/decade;
    double roundedFirstDecadeDomVal;

    if ( cvf::Math::abs(firstDecadeDomVal - cvf::Math::floor(firstDecadeDomVal)) < cvf::Math::abs(ceil(firstDecadeDomVal) - firstDecadeDomVal))
    {
        roundedFirstDecadeDomVal = cvf::Math::floor(firstDecadeDomVal);
    }
    else
    {
        roundedFirstDecadeDomVal = ceil(firstDecadeDomVal);
    }

    double newStep = decade*(roundedFirstDecadeDomVal)/decadeParts;
    return newStep;
}

//--------------------------------------------------------------------------------------------------
/// Calculates a set of humanly readable levels. Works very well for linear, and ok for logarithmic.
/// The logarithmic needs a bit more tweaking, so should override this method for linear but not yet done.
//--------------------------------------------------------------------------------------------------
void ScalarMapperRangeBased::majorTickValues( std::vector<double>* domainValues) const
{
    CVF_ASSERT(domainValues != NULL);
    CVF_ASSERT(m_rangeMin != cvf::UNDEFINED_DOUBLE && m_rangeMax != cvf::UNDEFINED_DOUBLE);

    if (m_userDefinedLevelValues.empty())
    {
        domainValues->push_back(domainValue(0));
        if (m_levelCount > 1)
        {
            double stepSizeNorm = 1.0/static_cast<double>(m_levelCount);
            size_t i;

            if (m_adjustLevels) // adjust levels
            {
                double prevDomValue =  domainValue(0);
                for (i = 1; i < m_levelCount + 5; ++i)
                {
                    double prevNormPos = normalizedValue(prevDomValue);
                    double newNormPos = prevNormPos + stepSizeNorm;

                    double domValue = domainValue(newNormPos);
                    double domStep = domValue - prevDomValue;
                    double newLevel;

                    //newLevel = prevDomValue + adjust(domStep, domStep, m_decadeLevelCount);
                    newLevel = domValue;

                    // Must handle first level specially to get a good absolute staring point
                    // For log domain this must be done all the time, and it does not hamper linear, so.. do it always
                    newLevel = adjust(newLevel, domStep, m_decadeLevelCount);

                    if (normalizedValue(newLevel) > 1.0 - stepSizeNorm*0.4) break;

                    if (newLevel != prevDomValue) domainValues->push_back(newLevel);

                    prevDomValue = newLevel;
                }
            }
            else
            {
                double prevDomValue =  domainValue(0);
                for (i = 1; i < m_levelCount; ++i)
                {
                    double newLevel = domainValue(stepSizeNorm*static_cast<double>(i));

                    if (newLevel != prevDomValue)  domainValues->push_back(newLevel);
                    
                    prevDomValue = newLevel;
                }
            }
        }
        domainValues->push_back(domainValue(1));
    }
    else
    {
        // Use the user defined levels between max and min.
        // (max and min values are set from the user defined levels if not set explicitly)
        domainValues->push_back(m_rangeMin);

        std::set<double>::iterator it; 
        for (it = m_userDefinedLevelValues.begin(); it != m_userDefinedLevelValues.end(); ++it)
        {
            if (*it <= m_rangeMin  ) continue;
            if (*it >= m_rangeMax  ) continue;

            domainValues->push_back(*it);
        }
        domainValues->push_back(m_rangeMax);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Color3ub ScalarMapperRangeBased::colorFromUserColorGradient(double normalizedValue) const
{
    CVF_TIGHT_ASSERT(0.0 <= normalizedValue && normalizedValue <= 1.0);

    size_t colorIdx = static_cast<size_t>(normalizedValue * (m_textureSize - 1));

    CVF_TIGHT_ASSERT(colorIdx <  m_interpolatedUserGradientColors.size());
    return m_interpolatedUserGradientColors[colorIdx];
}


} // namespace cvf
