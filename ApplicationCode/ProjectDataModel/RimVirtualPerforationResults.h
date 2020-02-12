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

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <vector>

class RimRegularLegendConfig;

//==================================================================================================
///
///
//==================================================================================================
class RimVirtualPerforationResults : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimVirtualPerforationResults();
    ~RimVirtualPerforationResults() override;

    bool                    showConnectionFactors() const;
    bool                    showConnectionFactorsOnClosedConnections() const;
    double                  geometryScaleFactor() const;
    RimRegularLegendConfig* legendConfig() const;
    void                    loadData();

private:
    void                 fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    caf::PdmFieldHandle* objectToggleField() override;
    void                 defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                 initAfterRead() override;

private:
    caf::PdmField<bool>   m_isActive;
    caf::PdmField<bool>   m_showClosedConnections;
    caf::PdmField<double> m_geometryScaleFactor;

    caf::PdmChildField<RimRegularLegendConfig*> m_legendConfig;
};
