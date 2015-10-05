/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <cstddef>


//==================================================================================================
///  
///  
//==================================================================================================
class RimWellLogExtractionCurveImpl
{
public:
    static void validValuesIntervals(const std::vector<double>& values, std::vector< std::pair<size_t, size_t> >& intervals);
    static void validDepthValuesIntervals(const std::vector<double>& depthValues, size_t startIdx, size_t stopIdx, std::vector< std::pair<size_t, size_t> >& intervals);
    static void validCurvePointIntervals(const std::vector<double>& depthValues, const std::vector<double>& values, std::vector< std::pair<size_t, size_t> >& intervals);
    static void addValuesFromIntervals(const std::vector<double>& values, std::vector< std::pair<size_t, size_t> >& intervals, std::vector<double>* filteredValues);
    static void filteredIntervals(const std::vector< std::pair<size_t, size_t> >& intervals, std::vector< std::pair<size_t, size_t> >* fltrIntervals);
};


