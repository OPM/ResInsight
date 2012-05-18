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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfArray.h"
#include "cvfScalarMapper.h"
#include <vector>

namespace cvf {

//==================================================================================================
//
// Abstract base class for scalar mappers that communicate with a legend
//
//==================================================================================================
class LegendScalarMapper : public ScalarMapper
{
public:
    // Return a the set of domain values representing sensible major tickmarks
    virtual void   majorLevels(std::vector<double>* domainValues) const = 0;  
    // Return the normalized (0.0, 1.0) representation of the domainValue
    virtual double normalizedLevelPosition(double domainValue) const = 0;
    // Return the domain value from a normalized val
    virtual double domainValue(double normalizedPosition) const = 0;

};


}
