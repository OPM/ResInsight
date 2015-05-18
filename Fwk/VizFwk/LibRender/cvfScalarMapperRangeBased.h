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


#pragma once

#include "cvfScalarMapper.h"

namespace cvf {

//==================================================================================================
//
// A base class that implements the common things between ordinary range based legends.
//
//==================================================================================================
class ScalarMapperRangeBased : public ScalarMapper
{
public:
    ScalarMapperRangeBased();

    // Public interface for setting up the mapping
    void                setRange(double min, double max);
    void                setLevelCount(size_t colorCount, bool adjustLevels);
    void                setLevelsFromValues(const std::set<double>& levelValues);

    void                setColors(const Color3ubArray& colorArray);
    void                setColors(ColorTable colorTable);

    // Implementing some of the Scalarmapper interface
    virtual Vec2f       mapToTextureCoord(double scalarValue) const;
    virtual Color3ub    mapToColor(double scalarValue) const;
    virtual bool        updateTexture(TextureImage* image) const;
    virtual void        majorTickValues(std::vector<double>* domainValues ) const;
    
protected:
    virtual void        rangeUpdated() {}; //< Called when the range is changed. Subclasses can reimplment to recalculate cached values
    Color3ub            colorFromUserColorGradient(double normalizedValue) const;

    double              m_rangeMin;
    double              m_rangeMax;
    unsigned int        m_decadeLevelCount;
    std::set<double>    m_sortedLevels; 

private:
    void                updateSortedLevels();

private:
    size_t              m_levelCount;       //< Number of discrete colors between min and max or number of sections between major ticks
    bool                m_adjustLevels;     //< Toggles wether to round tick positions to nice numbers
    std::set<double>    m_userDefinedLevelValues;

    Color3ubArray       m_interpolatedUserGradientColors; //< Table of smooth interpolated colors as a result of the users color request
    uint                m_textureSize;      // The size of texture that updateTexture() will produce. 
};

}
