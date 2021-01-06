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
class RimViewNameConfig : public RimNameConfig
{
    CAF_PDM_HEADER_INIT;

public:
    explicit RimViewNameConfig();

    void setAddCaseName( bool add );
    bool addCaseName() const;
    void setAddAggregationType( bool add );
    bool addAggregationType() const;
    void setAddProperty( bool add );
    bool addProperty() const;
    void setAddSampleSpacing( bool add );
    bool addSampleSpacing() const;

    void hideCaseNameField( bool hide );
    void hideAggregationTypeField( bool hide );
    void hidePropertyField( bool hide );
    void hideSampleSpacingField( bool hide );

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    void doEnableAllAutoNameTags( bool enable ) override;

private:
    caf::PdmField<bool> m_addCaseName;
    caf::PdmField<bool> m_addAggregationType;
    caf::PdmField<bool> m_addProperty;
    caf::PdmField<bool> m_addSampleSpacing;

    bool m_hideCaseNameField;
    bool m_hideAggregationTypeField;
    bool m_hidePropertyField;
    bool m_hideSampleSpacingField;
};
