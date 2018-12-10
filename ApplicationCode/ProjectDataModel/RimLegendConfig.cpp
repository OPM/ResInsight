/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Statoil ASA
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

#include "RimLegendConfig.h"

// NB! Special macro for pure virtual class
CAF_PDM_XML_ABSTRACT_SOURCE_INIT(RimLegendConfig, "LegendConfig");

namespace caf {
    template<>
    void AppEnum<RimLegendConfig::RangeModeType>::setUp()
    {
        addItem(RimLegendConfig::AUTOMATIC_ALLTIMESTEPS,     "AUTOMATIC_ALLTIMESTEPS",      "Min and Max for All Timesteps");
        addItem(RimLegendConfig::AUTOMATIC_CURRENT_TIMESTEP, "AUTOMATIC_CURRENT_TIMESTEP",  "Min and Max for Current Timestep");
        addItem(RimLegendConfig::USER_DEFINED,               "USER_DEFINED_MAX_MIN",        "User Defined Range");
        setDefault(RimLegendConfig::AUTOMATIC_ALLTIMESTEPS);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimLegendConfig::RimLegendConfig()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimLegendConfig::~RimLegendConfig()
{

}
