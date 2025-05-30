/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

class RimEclipseCellColors;
class RimEclipseView;

//==================================================================================================
///
///
//==================================================================================================
class RimEclipseFaultColors : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimEclipseFaultColors();
    ~RimEclipseFaultColors() override;

    void setReservoirView( RimEclipseView* ownerReservoirView );

    caf::PdmField<bool> showCustomFaultResult;

    bool                  hasValidCustomResult();
    RimEclipseCellColors* customFaultResult();

    void updateUiFieldsFromActiveResult();

protected:
    void                 initAfterRead() override;
    caf::PdmFieldHandle* objectToggleField() override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

private:
    caf::PdmChildField<RimEclipseCellColors*> m_customFaultResultColors;
    caf::PdmPointer<RimEclipseView>           m_reservoirView;
};
