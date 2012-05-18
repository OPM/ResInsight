//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#include "cvfScalarMapperDiscreteLinear.h"
#include "cvfMath.h"
#include "cvfTextureImage.h"
#include <cmath>

namespace cvf {
//==================================================================================================
///
/// \class cvf::ScalarMapperDiscreteLinear
/// \ingroup Render
///
/// Maps scalar values to texture coordinates/colors using discrete linear mapping
//==================================================================================================

ScalarMapperDiscreteLinear::ScalarMapperDiscreteLinear()
:   m_rangeMin(cvf::UNDEFINED_DOUBLE),
    m_rangeMax(cvf::UNDEFINED_DOUBLE),
    m_decadeLevelCount(1),
    m_colorCount(8),
    m_textureSize(2048) // Large enough, I guess and a power of two
{
    m_colors.resize(m_textureSize);
    m_colors.setAll(Color3ub::WHITE);
    setColors(ScalarMapper::NORMAL);

}


//--------------------------------------------------------------------------------------------------
/// Sets the max and min level of the legend. If the levels previously has been set with setLevelsFromValues() 
/// only the values between the new max min range becomes visible.
//--------------------------------------------------------------------------------------------------
void ScalarMapperDiscreteLinear::setRange(double min, double max)
{
    m_rangeMin = min;
    m_rangeMax = max;
    updateSortedLevels();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ScalarMapperDiscreteLinear::setColors(const Color3ubArray& colorArray)
{
    m_colors = *interpolateColorArray(colorArray, m_textureSize);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ScalarMapperDiscreteLinear::setColors(ColorTable colorTable)
{
    ref<Color3ubArray> baseColors = colorTableArray(colorTable);
    setColors(*baseColors);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Vec2f ScalarMapperDiscreteLinear::mapToTextureCoord(double scalarValue) const
{
    double discVal = discretize(scalarValue);
    return Vec2f(static_cast<float>(normalizedLevelPosition(discVal)), 0.5f);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double ScalarMapperDiscreteLinear::discretize(double scalarValue) const
{
    std::set<double>::iterator it;

    it = m_sortedLevels.upper_bound(scalarValue);
    if (it == m_sortedLevels.begin()) return (*it);
    if (it == m_sortedLevels.end()) return (*m_sortedLevels.rbegin());
    double upperValue = *it;
    it--;
    double lowerValue = *it;
    return 0.5 * (upperValue + lowerValue);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ScalarMapperDiscreteLinear::updateSortedLevels()
{
    std::vector<double> levels;
    majorLevels(&levels);
    std::set<double>::iterator it;
    m_sortedLevels.clear();
    m_sortedLevels.insert(levels.begin(), levels.end());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color3ub ScalarMapperDiscreteLinear::mapToColor(double scalarValue) const
{
    double discVal = discretize(scalarValue);

    size_t colorIdx = static_cast<size_t>(normalizedLevelPosition(discVal) * (m_textureSize - 1));

    CVF_TIGHT_ASSERT(colorIdx <  m_colors.size());
    return m_colors[colorIdx];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ScalarMapperDiscreteLinear::updateTexture(TextureImage* image) const
{
    CVF_ASSERT(image);

    image->allocate(m_textureSize, 1);

    // For now fill with white so we can see any errors more easily
    image->fill(Color4ub(Color3::WHITE));

    uint ic;
    for (ic = 0; ic < m_textureSize; ic++)
    {
        const Color4ub clr(m_colors[ic], 255);
        image->setPixel(ic, 0, clr);
    }
 
    return true;
}

// Then calculate a stepsize that is humanly understandable
// basically rounded to whole or half of the decade in question

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
void ScalarMapperDiscreteLinear::majorLevels( std::vector<double>* domainValues) const
{
    CVF_ASSERT(domainValues != NULL);
    CVF_ASSERT(m_rangeMin != cvf::UNDEFINED_DOUBLE && m_rangeMax != cvf::UNDEFINED_DOUBLE);

    if (m_userDefinedLevelValues.empty())
    {
        domainValues->push_back(m_rangeMin);
        if (m_colorCount > 1)
        {
            double stepSizeNorm = 1.0/m_colorCount;
            size_t i;

            if (m_adjustLevels) // adjust levels
            {
                double prevDomValue =  domainValue(0);
                for (i = 1; i < m_colorCount + 5; ++i)
                {
                    double prevNormPos = normalizedLevelPosition(prevDomValue);
                    double newNormPos = prevNormPos + stepSizeNorm;
                    double domValue = domainValue(newNormPos);
                    double domStep = domValue - prevDomValue;
                    double newLevel;

                    newLevel = prevDomValue + adjust(domStep, domStep, m_decadeLevelCount);

                    // Must handle first level specially to get a good absolute staring point
                    // For log domain this must be done all the time, and it does not hamper linear, so.. do it always
                    newLevel = adjust(newLevel, domStep, m_decadeLevelCount);

                    if (newLevel > m_rangeMax - domStep*0.4) break;

                    domainValues->push_back(newLevel);
                    prevDomValue = newLevel;
                }
            }
            else
            {
                for (i = 1; i < m_colorCount; ++i)
                {
                    domainValues->push_back(domainValue(stepSizeNorm*i));
                }
            }
        }
        domainValues->push_back(m_rangeMax);
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
double ScalarMapperDiscreteLinear::normalizedLevelPosition(double domainScalarValue) const
{
    double range = m_rangeMax - m_rangeMin;
    if (range != 0) return cvf::Math::clamp((domainScalarValue - m_rangeMin)/range, 0.0, 1.0);
    else            return 0;
} 

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double ScalarMapperDiscreteLinear::domainValue(double normalizedPosition) const
{
    double range = m_rangeMax - m_rangeMin;

    return (normalizedPosition * range + m_rangeMin);

}

//--------------------------------------------------------------------------------------------------
/// Sets the number of levels to 1 + colorCount making the scalar mapper have colorCount 
/// distinct visible colors
//--------------------------------------------------------------------------------------------------
void ScalarMapperDiscreteLinear::setLevelsFromColorCount(size_t colorCount, bool adjustLevels)
{
    m_userDefinedLevelValues.clear();

    m_colorCount = colorCount;
    m_adjustLevels = adjustLevels;

    updateSortedLevels();
}

//--------------------------------------------------------------------------------------------------
/// This method sets all the levels to the user defined domain values, 
/// overriding any previous max and min range settings.
//--------------------------------------------------------------------------------------------------
void ScalarMapperDiscreteLinear::setLevelsFromValues(const std::set<double>& levelValues)
{
    CVF_ASSERT(!m_userDefinedLevelValues.empty());

    m_userDefinedLevelValues = levelValues;
    m_rangeMax = (*levelValues.rbegin());
    m_rangeMin = (*levelValues.begin());
    updateSortedLevels();
}


} // namespace cvf
