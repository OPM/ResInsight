/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimUserDefinedCalculationVariable.h"

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

#include "RifEclipseSummaryAddressQMetaType.h"

class RimSummaryCase;
class RimSummaryAddress;
class RiuSummaryVectorSelectionDialog;

//==================================================================================================
///
///
//==================================================================================================
class RimSummaryCalculationVariable : public RimUserDefinedCalculationVariable
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCalculationVariable();

    QString displayString() const override;

    RimSummaryCase*    summaryCase();
    RimSummaryAddress* summaryAddress();

    void setSummaryAddress( const RimSummaryAddress& address );

    void handleDroppedMimeData( const QMimeData* data, Qt::DropAction action, caf::PdmFieldHandle* destinationField ) override;

    RimSummaryCalculationVariable* clone() const;

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    void readDataFromApplicationStore( RiuSummaryVectorSelectionDialog* selectionDialog ) const;
    void writeDataToApplicationStore() const;

private:
    caf::PdmField<bool> m_button;

    caf::PdmPtrField<RimSummaryCase*>      m_case;
    caf::PdmChildField<RimSummaryAddress*> m_summaryAddress;
};
