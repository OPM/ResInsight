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

#include "RimExportWellSegmentsSettings.h"

namespace caf {
    template<>
    void RimExportWellSegmentsSettings::PressureDropEnum::setUp()
    {
        addItem(RimExportWellSegmentsSettings::HYDROSTATIC,                       "H--", "Hydrostatic");
        addItem(RimExportWellSegmentsSettings::HYDROSTATIC_FRICTION,              "HF-", "Hydrostatic + Friction");
        addItem(RimExportWellSegmentsSettings::HYDROSTATIC_FRICTION_ACCELERATION, "HFA", "Hydrostatic + Friction + Acceleration");
        setDefault(RimExportWellSegmentsSettings::HYDROSTATIC);
    }

    template<>
    void RimExportWellSegmentsSettings::LengthAndDepthEnum::setUp()
    {
        addItem(RimExportWellSegmentsSettings::INC, "INC", "Incremental");
        addItem(RimExportWellSegmentsSettings::ABS, "ABS", "Absolute");
        setDefault(RimExportWellSegmentsSettings::INC);
    }
}

CAF_PDM_SOURCE_INIT(RimExportWellSegmentsSettings, "RimExportWellSegmentsSettings");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimExportWellSegmentsSettings::RimExportWellSegmentsSettings()
{
    CAF_PDM_InitObject("RimExportWellSegmentsSettings", "", "", "");

    CAF_PDM_InitFieldNoDefault(&pressureDrop, "PressureDrop", "Pressure Drop", "", "", "");
    CAF_PDM_InitFieldNoDefault(&lengthAndDepth, "LengthAndDepth", "Length and Depth", "", "", "");
}
