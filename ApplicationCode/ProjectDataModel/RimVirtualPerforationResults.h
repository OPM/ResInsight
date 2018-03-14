/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Statoil ASA
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

class RimLegendConfig;

//==================================================================================================
///  
///  
//==================================================================================================
class RimVirtualPerforationResults : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimVirtualPerforationResults();
    virtual ~RimVirtualPerforationResults();

    bool    isActive() const;
    double  geometryScaleFactor() const;
    RimLegendConfig* legendConfig() const;

private:
    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual caf::PdmFieldHandle*            objectToggleField() override;
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:
    caf::PdmField<bool>                     m_isActive;
    caf::PdmField<double>                   m_geometryScaleFactor;

    caf::PdmChildField<RimLegendConfig*>    m_legendConfig;
};
