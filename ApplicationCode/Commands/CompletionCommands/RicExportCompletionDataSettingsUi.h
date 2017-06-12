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
class RicExportCompletionDataSettingsUi : public RicCaseAndFileExportSettingsUi
{
    CAF_PDM_HEADER_INIT;
public:
    RicExportCompletionDataSettingsUi();


    caf::PdmField<bool>                     computeTransmissibility;
    caf::PdmField<bool>                     includePerforations;
    caf::PdmField<bool>                     includeFishbones;
    caf::PdmField<bool>                     includeFractures;

    caf::PdmField<bool>                     includeWpimult;
    caf::PdmField<bool>                     removeLateralsInMainBoreCells;

    caf::PdmField<int>                      timeStep;

    void                                    showForSimWells();
    void                                    showForWellPath();

protected:
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:
    bool                                    m_displayForSimWell;
};
