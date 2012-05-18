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

namespace cvf {

class GeometryBuilder;

//==================================================================================================
//
// Generates an arrow with base in origin and pointing in the +Z direction
//
//==================================================================================================
class ArrowGenerator
{
public:
    ArrowGenerator();

    void setShaftRelativeRadius(float shaftRelativeRadius);
    void setHeadRelativeRadius(float headRelativeRadius);
    void setHeadRelativeLength(float headRelativeLength);

    void setNumSlices(uint numSlices);

    void generate(GeometryBuilder* builder);

private:
    float m_shaftRelativeRadius;
    float m_headRelativeRadius;
    float m_headRelativeLength;

    uint m_numSlices;
};

}
