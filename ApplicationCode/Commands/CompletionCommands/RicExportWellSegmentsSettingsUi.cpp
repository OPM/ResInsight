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

#include "RicExportWellSegmentsSettingsUi.h"

namespace caf {
    template<>
    void RicExportWellSegmentsSettingsUi::PressureDropEnum::setUp()
    {
        addItem(RicExportWellSegmentsSettingsUi::HYDROSTATIC,                       "H--", "Hydrostatic");
        addItem(RicExportWellSegmentsSettingsUi::HYDROSTATIC_FRICTION,              "HF-", "Hydrostatic + Friction");
        addItem(RicExportWellSegmentsSettingsUi::HYDROSTATIC_FRICTION_ACCELERATION, "HFA", "Hydrostatic + Friction + Acceleration");
        setDefault(RicExportWellSegmentsSettingsUi::HYDROSTATIC);
    }

    template<>
    void RicExportWellSegmentsSettingsUi::LengthAndDepthEnum::setUp()
    {
        addItem(RicExportWellSegmentsSettingsUi::INC, "INC", "Incremental");
        addItem(RicExportWellSegmentsSettingsUi::ABS, "ABS", "Absolute");
        setDefault(RicExportWellSegmentsSettingsUi::INC);
    }
}

CAF_PDM_SOURCE_INIT(RicExportWellSegmentsSettingsUi, "RicExportWellSegmentsSettingsUi");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicExportWellSegmentsSettingsUi::RicExportWellSegmentsSettingsUi()
{
    CAF_PDM_InitObject("RimExportWellSegmentsSettings", "", "", "");

    CAF_PDM_InitFieldNoDefault(&pressureDrop, "PressureDrop", "Pressure Drop", "", "", "");
    CAF_PDM_InitFieldNoDefault(&lengthAndDepth, "LengthAndDepth", "Length and Depth", "", "", "");
}
