/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017  Statoil ASA
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

#include "RimTimeStepFilter.h"

CAF_PDM_SOURCE_INIT(RimTimeStepFilter, "TimeStepFilter");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimTimeStepFilter::RimTimeStepFilter()
{
    CAF_PDM_InitObject("Time Step Filter", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_timeStepIndicesToImport, "TimeStepIndicesToImport", "Values", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RimTimeStepFilter::timeStepIndicesToImport() const
{
    std::vector<size_t> indices;

    // Convert vector from int to size_t
    for (auto intValue : m_timeStepIndicesToImport.v())
    {
        indices.push_back(intValue);
    }

    return indices;
}
