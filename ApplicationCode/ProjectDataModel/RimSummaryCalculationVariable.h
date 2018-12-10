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

#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cafPdmPtrField.h"
#include "cafPdmProxyValueField.h"

#include "RifEclipseSummaryAddressQMetaType.h"
#include "cafPdmChildField.h"

class RimSummaryCase;
class RimSummaryAddress;
class RiuSummaryCurveDefSelectionDialog;

//==================================================================================================
///  
///  
//==================================================================================================
class RimSummaryCalculationVariable : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCalculationVariable();

    QString         name() const;
    void            setName(const QString& name);

    QString         summaryAddressDisplayString() const;

    RimSummaryCase*     summaryCase();
    RimSummaryAddress*  summaryAddress();

private:
    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void defineObjectEditorAttribute(QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;

    void readDataFromApplicationStore(RiuSummaryCurveDefSelectionDialog* selectionDialog) const;
    void writeDataToApplicationStore() const;

private:
    caf::PdmField<QString>                  m_name;
    caf::PdmProxyValueField<QString>        m_summaryAddressUi;

    caf::PdmField<bool>                     m_button;

    caf::PdmPtrField<RimSummaryCase*>       m_case;
    caf::PdmChildField<RimSummaryAddress*>  m_summaryAddress;
};
