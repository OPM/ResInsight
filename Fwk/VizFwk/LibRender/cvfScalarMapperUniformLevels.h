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

#include "cvfObject.h"
#include "cvfScalarMapper.h"

namespace cvf {

class OverlayColorLegend;


//==================================================================================================
//
// Maps scalar values to texture coordinates/colors
//
//==================================================================================================
class ScalarMapperUniformLevels : public ScalarMapper
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

    virtual Vec2f       mapToTextureCoord(double scalarValue) const;
    virtual Color3ub    mapToColor(double scalarValue) const;

    virtual bool        updateTexture(TextureImage* image) const;
    bool                updateColorLegend(OverlayColorLegend* legend) const;

    // HACK to compile until we reconcile new and old scheme for scalar mappers
    virtual void        majorTickValues(std::vector<double>*) const { CVF_FAIL_MSG("Not implemented"); }
    virtual double      normalizedValue(double) const               { CVF_FAIL_MSG("Not implemented"); return 0; }
    virtual double      domainValue(double) const                   { CVF_FAIL_MSG("Not implemented"); return 0; }

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
