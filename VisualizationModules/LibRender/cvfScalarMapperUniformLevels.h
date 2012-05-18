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

#pragma once

#include "cvfObject.h"
#include "cvfLegendScalarMapper.h"

namespace cvf {

class OverlayColorLegend;


//==================================================================================================
//
// Maps scalar values to texture coordinates/colors
//
//==================================================================================================
class ScalarMapperUniformLevels : public LegendScalarMapper
{
public:
    ScalarMapperUniformLevels();

    void                setRange(double min, double max);
    double              rangeMin() const;
    double              rangeMax() const;

    void                setColors(const Color3ubArray& colorArray);
    void                setColors(ColorTable colorTable, uint levelCount);

    void                setTextureSize(uint textureSize);
    uint                textureSize() const;

    // Scalarmapper interface
    virtual Vec2f       mapToTextureCoord(double scalarValue) const;
    virtual Color3ub    mapToColor(double scalarValue) const;

    virtual bool        updateTexture(TextureImage* image) const;

    // LegendScalarmapper interface

    virtual void        majorLevels(std::vector<double>* domainValues ) const;
    virtual double      normalizedLevelPosition( double domainValue ) const;
    virtual double      domainValue( double normalizedPosition ) const;

private:
    void                recomputeMaxTexCoord();



private:
    double          m_rangeMin;
    double          m_rangeMax;
    Color3ubArray   m_colors;

    uint            m_textureSize;      // The size of texture that updateTexture() is will produce. 
    double          m_maxTexCoord;      // The largest allowable s texture coordinate, scalar values >= m_rangeMax will get mapped to this coordinate
};


}
