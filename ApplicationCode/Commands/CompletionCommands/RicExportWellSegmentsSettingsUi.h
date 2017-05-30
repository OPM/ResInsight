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

#pragma once

#include "RicCaseAndFileExportSettingsUi.h"

#include "cafPdmField.h"

//==================================================================================================
///  
///  
//==================================================================================================
class RicExportWellSegmentsSettingsUi : public RicCaseAndFileExportSettingsUi
{
    CAF_PDM_HEADER_INIT;
public:

    enum PressureDropType {
        HYDROSTATIC,
        HYDROSTATIC_FRICTION,
        HYDROSTATIC_FRICTION_ACCELERATION
    };

    typedef caf::AppEnum<RicExportWellSegmentsSettingsUi::PressureDropType> PressureDropEnum;

    enum LengthAndDepthType {
        ABS,
        INC
    };

    typedef caf::AppEnum<RicExportWellSegmentsSettingsUi::LengthAndDepthType> LengthAndDepthEnum;

    RicExportWellSegmentsSettingsUi();

    caf::PdmField<PressureDropEnum>         pressureDrop;
    caf::PdmField<LengthAndDepthEnum>       lengthAndDepth;
};
