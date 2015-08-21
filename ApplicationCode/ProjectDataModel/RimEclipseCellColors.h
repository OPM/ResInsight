/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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
#include "RimEclipseResultDefinition.h"

class RimTernaryLegendConfig;

//==================================================================================================
///  
///  
//==================================================================================================
class RimEclipseCellColors :  public RimEclipseResultDefinition
{
    CAF_PDM_HEADER_INIT;
public:
    RimEclipseCellColors();
    virtual ~RimEclipseCellColors();

    virtual void setReservoirView(RimEclipseView* ownerReservoirView);
    caf::PdmField<RimLegendConfig*> legendConfig;
    caf::PdmField<RimTernaryLegendConfig*> ternaryLegendConfig;

    // Overridden methods
    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void setResultVariable(const QString& resultName);

protected:
    friend class RimEclipseFaultColors;
    virtual void initAfterRead();

private:
    void changeLegendConfig(QString resultVarNameOfNewLegend);

    caf::PdmField<std::list<caf::PdmPointer<RimLegendConfig> > >    m_legendConfigData;
};

