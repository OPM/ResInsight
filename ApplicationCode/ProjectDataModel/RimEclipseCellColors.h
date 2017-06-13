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

#include "RiaDefines.h"
#include "RimEclipseResultDefinition.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmPtrField.h"

class RimTernaryLegendConfig;
class RimLegendConfig;

//==================================================================================================
///  
///  
//==================================================================================================
class RimEclipseCellColors : public RimEclipseResultDefinition
{
    CAF_PDM_HEADER_INIT;
public:
    RimEclipseCellColors();
    virtual ~RimEclipseCellColors();

    void                                        setReservoirView(RimEclipseView* ownerReservoirView);
    RimEclipseView*                             reservoirView();

    void                                        updateLegendData(size_t timestep);
    RimLegendConfig*                            legendConfig();
    caf::PdmChildField<RimTernaryLegendConfig*> ternaryLegendConfig;

    virtual void                                setResultVariable(const QString& resultName);
    
    void                                        updateIconState();

    virtual void                                updateLegendCategorySettings() override;

protected:
    // Overridden methods
    virtual void                                fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void                                defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "") override;

    friend class RimEclipseFaultColors;
    friend class RimCellEdgeColors;
    virtual void                                initAfterRead();

private:
    void                                        changeLegendConfig(QString resultVarNameOfNewLegend);

    caf::PdmChildArrayField<RimLegendConfig*>   m_legendConfigData;
    caf::PdmPtrField<RimLegendConfig*>          m_legendConfigPtrField;

    caf::PdmPointer<RimEclipseView>             m_reservoirView;

    // Obsolete   
    caf::PdmChildField<RimLegendConfig*>        obsoleteField_legendConfig;
};

