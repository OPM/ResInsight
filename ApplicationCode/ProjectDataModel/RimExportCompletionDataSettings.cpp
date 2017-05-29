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

#include "RimExportCompletionDataSettings.h"

namespace caf {
    template<>
    void RimExportCompletionDataSettings::PressureDropEnum::setUp()
    {
        addItem(RimExportCompletionDataSettings::HYDROSTATIC,                       "H--", "Hydrostatic");
        addItem(RimExportCompletionDataSettings::HYDROSTATIC_FRICTION,              "HF-", "Hydrostatic + Friction");
        addItem(RimExportCompletionDataSettings::HYDROSTATIC_FRICTION_ACCELERATION, "HFA", "Hydrostatic + Friction + Acceleration");
        setDefault(RimExportCompletionDataSettings::HYDROSTATIC);
    }

    template<>
    void RimExportCompletionDataSettings::LengthAndDepthEnum::setUp()
    {
        addItem(RimExportCompletionDataSettings::INC, "INC", "Incremental");
        addItem(RimExportCompletionDataSettings::ABS, "ABS", "Absolute");
        setDefault(RimExportCompletionDataSettings::INC);
    }
}

CAF_PDM_SOURCE_INIT(RimExportCompletionDataSettings, "RimExportCompletionDataSettings");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimExportCompletionDataSettings::RimExportCompletionDataSettings()
{
    CAF_PDM_InitObject("RimExportCompletionDataSettings", "", "", "");

    CAF_PDM_InitField(&includePerforations, "IncludePerforations", true, "Include Perforations", "", "", "");
    CAF_PDM_InitField(&includeFishbones, "IncludeFishbones", true, "Include Fishbones", "", "", "");

    CAF_PDM_InitField(&includeWpimult, "IncludeWPIMULT", true, "Include WPIMLUT", "", "", "");
    CAF_PDM_InitField(&removeLateralsInMainBoreCells, "RemoveLateralsInMainBoreCells", false, "Remove Laterals in Main Bore Cells", "", "", "");
    CAF_PDM_InitFieldNoDefault(&pressureDrop, "PressureDrop", "Pressure Drop", "", "", "");
    CAF_PDM_InitFieldNoDefault(&lengthAndDepth, "LengthAndDepth", "Length and Depth", "", "", "");
}
