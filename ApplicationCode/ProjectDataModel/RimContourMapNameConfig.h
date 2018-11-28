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
class RimContourMapNameConfig : public RimNameConfig
{
    CAF_PDM_HEADER_INIT;

public:
    RimContourMapNameConfig(const RimNameConfigHolderInterface* configHolder = nullptr);

    bool                     addCaseName() const;
    bool                     addAggregationType() const;
    bool                     addProperty() const;
    bool                     addSampleSpacing() const;

    void                     enableAllAutoNameTags(bool enable) override;

protected:
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:
    caf::PdmField<bool>              m_addCaseName;
    caf::PdmField<bool>              m_addAggregationType;
    caf::PdmField<bool>              m_addProperty;
    caf::PdmField<bool>              m_addSampleSpacing;
};


