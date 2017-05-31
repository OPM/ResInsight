/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include <string>
#include <set>

#define RIG_FLD_TOF_RESNAME                  "TOF"
#define RIG_FLD_CELL_FRACTION_RESNAME        "Fraction"
#define RIG_FLD_MAX_FRACTION_TRACER_RESNAME  "MaxFractionTracer"
#define RIG_FLD_COMMUNICATION_RESNAME        "Communication"

#define RIG_FLOW_TOTAL_NAME "Total"
#define RIG_FLOW_OIL_NAME "Oil"
#define RIG_FLOW_GAS_NAME "Gas"
#define RIG_FLOW_WATER_NAME "Water"

#define RIG_RESERVOIR_TRACER_NAME "Reservoir"
#define RIG_TINY_TRACER_GROUP_NAME "Other"

class RigFlowDiagResultAddress
{

public:
    RigFlowDiagResultAddress(const std::string& aVariableName, const std::set<std::string>& someSelectedTracerNames) 
    : variableName(aVariableName), selectedTracerNames(someSelectedTracerNames) {}

    RigFlowDiagResultAddress(const std::string& aVariableName, const std::string& tracerName)
    : variableName(aVariableName)
    {
        selectedTracerNames.insert(tracerName);
    }

    bool isNativeResult() const;

    std::string uiText() const;
    std::string uiShortText() const;

    std::string           variableName;
    std::set<std::string> selectedTracerNames;

    bool operator< (const RigFlowDiagResultAddress& other) const
    {
        if ( selectedTracerNames != other.selectedTracerNames )
        {
            return selectedTracerNames < other.selectedTracerNames;
        }

        return variableName < other.variableName;
    }
};

