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

#include "cvfScalarMapperRangeBased.h"

namespace cvf {

//==================================================================================================
//
// Maps scalar values to texture coordinates/colors
//
//==================================================================================================

class ScalarMapperDiscreteLog : public ScalarMapperRangeBased
{
public:
    ScalarMapperDiscreteLog();

    // Implementing the Scalarmapper interface
    virtual Vec2f       mapToTextureCoord(double scalarValue) const;
    virtual Color3ub    mapToColor(double scalarValue) const;
    virtual double      normalizedValue( double domainValue ) const;
    virtual double      domainValue( double normalizedPosition ) const;

    //
protected:
    virtual void rangeUpdated();

private:
    double  m_logRange;
    double  m_logRangeMin;
    bool    m_hasNegativeRange;
};
}
