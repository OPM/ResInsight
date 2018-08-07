/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RimNameConfig.h"

//==================================================================================================
///
///
//==================================================================================================
class RimWellLogPlotNameConfig : public RimNameConfig
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellLogPlotNameConfig(const RimNameConfigHolderInterface* configHolder = nullptr);
    virtual caf::PdmUiGroup* createUiGroup(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

    bool                     addCaseName() const;
    bool                     addWellName() const;
    bool                     addTimeStep() const;
    bool                     addAirGap() const;
    bool                     addWaterDepth() const;
    virtual void             enableAllAutoNameTags(bool enable) override;

private:
    caf::PdmField<bool>              m_addCaseName;
    caf::PdmField<bool>              m_addWellName;
    caf::PdmField<bool>              m_addTimestep;
    caf::PdmField<bool>              m_addAirGap;
    caf::PdmField<bool>              m_addWaterDepth;
};


