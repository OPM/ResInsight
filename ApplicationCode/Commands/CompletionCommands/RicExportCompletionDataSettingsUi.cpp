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

#include "RicExportCompletionDataSettingsUi.h"

CAF_PDM_SOURCE_INIT(RicExportCompletionDataSettingsUi, "RicExportCompletionDataSettingsUi");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicExportCompletionDataSettingsUi::RicExportCompletionDataSettingsUi()
{
    CAF_PDM_InitObject("RimExportCompletionDataSettings", "", "", "");

    CAF_PDM_InitField(&timeStep, "TimeStepIndex", 0, "Time Step", "", "", "");

    CAF_PDM_InitField(&computeTransmissibility, "ComputeTransmissibility", true, "Compute Transmissibility", "", "", "");

    CAF_PDM_InitField(&includePerforations, "IncludePerforations", true, "Include Perforations", "", "", "");
    CAF_PDM_InitField(&includeFishbones, "IncludeFishbones", true, "Include Fishbones", "", "", "");

    CAF_PDM_InitField(&includeWpimult, "IncludeWPIMULT", true, "Include WPIMLUT", "", "", "");
    CAF_PDM_InitField(&removeLateralsInMainBoreCells, "RemoveLateralsInMainBoreCells", false, "Remove Laterals in Main Bore Cells", "", "", "");

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RicExportCompletionDataSettingsUi::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;
    if (fieldNeedingOptions == &timeStep)
    {
        QStringList timeStepNames;

        if (caseToApply)
        {
            timeStepNames = caseToApply->timeStepStrings();
        }

        for (int i = 0; i < timeStepNames.size(); i++)
        {
            options.push_back(caf::PdmOptionItemInfo(timeStepNames[i], i));
        }
    }
    else
    {
        options = RicCaseAndFileExportSettingsUi::calculateValueOptions(fieldNeedingOptions, useOptionsOnly);
    }
    return options;
}
