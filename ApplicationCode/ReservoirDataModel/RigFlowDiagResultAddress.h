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

#include "cafAppEnum.h"

#include <string>
#include <set>

#define RIG_FLD_TOF_RESNAME                  "TOF"
#define RIG_FLD_CELL_FRACTION_RESNAME        "Fraction"
#define RIG_FLD_MAX_FRACTION_TRACER_RESNAME  "MaxFractionTracer"
#define RIG_FLD_COMMUNICATION_RESNAME        "Communication"
#define RIG_NUM_FLOODED_PV                   "Number of Flooded PV"

#define RIG_FLOW_TOTAL_NAME "Total"
#define RIG_FLOW_OIL_NAME "Oil"
#define RIG_FLOW_GAS_NAME "Gas"
#define RIG_FLOW_WATER_NAME "Water"

#define RIG_RESERVOIR_TRACER_NAME "Reservoir"
#define RIG_TINY_TRACER_GROUP_NAME "Other"

class RigFlowDiagResultAddress
{

public:
    enum PhaseSelection
    {
        PHASE_ALL = 0b111,
        PHASE_OIL = 0b001,
        PHASE_GAS = 0b010,
        PHASE_WAT = 0b100,
    };

    typedef caf::AppEnum<PhaseSelection> PhaseSelectionEnum;

    RigFlowDiagResultAddress(const std::string& aVariableName, PhaseSelection phaseSelection, const std::set<std::string>& someSelectedTracerNames) 
    : variableName(aVariableName),
      phaseSelection(phaseSelection),
      selectedTracerNames(someSelectedTracerNames) {}

    RigFlowDiagResultAddress(const std::string& aVariableName, PhaseSelection phaseSelection, const std::string& tracerName)
    : variableName(aVariableName),
      phaseSelection(phaseSelection)
    {
        selectedTracerNames.insert(tracerName);
    }

    bool isNativeResult() const;

    std::string uiText() const;
    std::string uiShortText() const;

    std::string           variableName;
    std::set<std::string> selectedTracerNames;
    PhaseSelection        phaseSelection;

    bool operator< (const RigFlowDiagResultAddress& other) const
    {
        if ( selectedTracerNames != other.selectedTracerNames )
        {
            return selectedTracerNames < other.selectedTracerNames;
        }
        if (phaseSelection != other.phaseSelection)
        {
            return phaseSelection < other.phaseSelection;
        }

        return variableName < other.variableName;
    }
};

