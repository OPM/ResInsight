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

#include "cvfScalarMapperContinuousLog.h"

#include "cvfMath.h"
#include "cvfTextureImage.h"
#include <cmath>

namespace cvf {

//==================================================================================================
///
/// \class cvf::ScalarMapperContinuousLog
/// \ingroup Render
///
/// Maps scalar values to texture coordinates/colors using continuous logarithmic mapping
/// Configured by specifying a number of level colors and a min/max range. 
//==================================================================================================
ScalarMapperContinuousLog::ScalarMapperContinuousLog()
:   m_rangeMin(1),
    m_rangeMax(1),
    m_decadeLevelCount(1),
    m_majorLevelCount(8),
    m_textureSize(2048) // Large enough, I guess and a power of two
{
    m_colors.resize(m_textureSize);
    m_colors.setAll(Color3ub::WHITE);
    setColors(ScalarMapper::NORMAL);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ScalarMapperContinuousLog::setRange(double min, double max)
{
    m_rangeMin = min;
    m_rangeMax = max;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ScalarMapperContinuousLog::setColors(const Color3ubArray& colorArray)
{
    m_colors = *interpolateColorArray(colorArray, m_textureSize);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ScalarMapperContinuousLog::setColors(ColorTable colorTable)
{
    ref<Color3ubArray> baseColors = colorTableArray(colorTable);
    setColors(*baseColors);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Vec2f ScalarMapperContinuousLog::mapToTextureCoord(double scalarValue) const
{
    return Vec2f(static_cast<float>(normalizedLevelPosition(scalarValue)), 0.5f);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Color3ub ScalarMapperContinuousLog::mapToColor(double scalarValue) const
{
    size_t colorIdx = static_cast<size_t>(normalizedLevelPosition(scalarValue) * (m_textureSize - 1));
    CVF_TIGHT_ASSERT(colorIdx <  m_colors.size());
    return m_colors[colorIdx];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ScalarMapperContinuousLog::updateTexture(TextureImage* image) const
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
void ScalarMapperContinuousLog::majorLevels( std::vector<double>* domainValues) const
{
    CVF_ASSERT(domainValues != NULL);

    domainValues->push_back(m_rangeMin);

    if (m_majorLevelCount > 1)
    {
        double stepSizeNorm = 1.0/m_majorLevelCount;
        size_t i;

        if (m_adjustLevels) // adjust levels
        {
            double prevDomValue =  domainValue(0);
            for (i = 1; i < m_majorLevelCount + 5; ++i)
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
            for (i = 1; i < m_majorLevelCount; ++i)
            {
                domainValues->push_back(domainValue(stepSizeNorm*i));
            }
        }
    }

    domainValues->push_back(m_rangeMax);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double ScalarMapperContinuousLog::normalizedLevelPosition(double scalarValue) const
{
    double logRangeMax = log10(m_rangeMax);
    double logRangeMin = log10(m_rangeMin);
    double logRange = logRangeMax - logRangeMin; 
    double logValue;
    
    if (scalarValue <= 0) logValue = logRangeMin;
    else                  logValue = log10(scalarValue);

   if (logRange != 0)
    {
        return cvf::Math::clamp((logValue - logRangeMin)/logRange, 0.0, 1.0);
    }
    else
    {
        return 0;
    }
} 

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double ScalarMapperContinuousLog::domainValue(double normalizedPosition) const
{
    double logRangeMax = log10(m_rangeMax);
    double logRangeMin = log10(m_rangeMin);
    double logRange = logRangeMax - logRangeMin; 

    double logValue = normalizedPosition*logRange + logRangeMin;
    
    return pow(10, logValue);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ScalarMapperContinuousLog::setMajorLevelCount(size_t levelCount, bool adjustLevels)
{
    m_majorLevelCount = levelCount;
    m_adjustLevels = adjustLevels;
}

} // namespace cvf
