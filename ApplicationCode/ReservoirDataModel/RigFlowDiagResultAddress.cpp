/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RigFlowDiagResultAddress.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigFlowDiagResultAddress::isNativeResult() const
{
    return (((variableName == RIG_FLD_TOF_RESNAME) || (variableName == RIG_FLD_CELL_FRACTION_RESNAME)) && selectedTracerNames.size() <= 1);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RigFlowDiagResultAddress::uiText() const
{
    std::string uiVarname = variableName;

    std::string uitext = uiVarname;
    uitext += " (";
    for (const std::string& tracerName : selectedTracerNames)
    {
        uitext += " " + tracerName;
    }
    uitext += " )";
    return uitext;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RigFlowDiagResultAddress::uiShortText() const
{
    return variableName;
}

