/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RimDefines.h"
#include "RimLegendConfig.h"
#include "RimResultDefinition.h"

class RimTernaryLegendConfig;

//==================================================================================================
///  
///  
//==================================================================================================
class RimResultSlot :  public RimResultDefinition
{
    CAF_PDM_HEADER_INIT;
public:
    RimResultSlot();
    virtual ~RimResultSlot();

    virtual void setReservoirView(RimReservoirView* ownerReservoirView);
    caf::PdmField<RimLegendConfig*> legendConfig;
    caf::PdmField<RimTernaryLegendConfig*> ternaryLegendConfig;

    // Overridden methods
    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void setResultVariable(const QString& resultName);

protected:
    friend class RimFaultResultSlot;
    virtual void initAfterRead();

private:
    void changeLegendConfig(QString resultVarNameOfNewLegend);

    caf::PdmField<std::list<caf::PdmPointer<RimLegendConfig> > >    m_legendConfigData;
};

