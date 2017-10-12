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

#include "RimCalculationVariable.h"

#include "RiaSummaryCurveDefinition.h"

#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"

#include "RiuSummaryCurveDefSelection.h"
#include "RiuSummaryCurveDefSelectionDialog.h"

#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTableView.h"


CAF_PDM_SOURCE_INIT(RimCalculationVariable, "RimCalculationVariable");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCalculationVariable::RimCalculationVariable()
{
    CAF_PDM_InitObject("RimCalculationVariable", ":/octave.png", "RimCalculationVariable", "");

    CAF_PDM_InitFieldNoDefault(&m_name,             "VariableName",     "Variable Name", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_button, "PushButton", "Edit ", "", "", "");
    m_button.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_button.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault(&m_summaryAddressUi,   "SummaryAddressUi",   "Summary Address", "", "", "");
    m_summaryAddressUi.registerGetMethod(this, &RimCalculationVariable::summaryAddressDisplayString);
    m_summaryAddressUi.xmlCapability()->disableIO();
    
    CAF_PDM_InitFieldNoDefault(&m_case, "SummaryCase", "Summary Case", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_summaryAddress, "SummaryAddress", "Summary Address", "", "", "");
    m_summaryAddress = new RimSummaryAddress;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCalculationVariable::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_button)
    {
        RiuSummaryCurveDefSelectionDialog dlg(nullptr);
        {
            std::vector<RiaSummaryCurveDefinition> sumCasePairs;
            sumCasePairs.push_back(RiaSummaryCurveDefinition(m_case(), m_summaryAddress->address()));

            dlg.summaryAddressSelection()->setSelectedCurveDefinitions(sumCasePairs);
            dlg.summaryAddressSelection()->updateConnectedEditors();
        }

        if (dlg.exec() == QDialog::Accepted)
        {
            std::vector<RiaSummaryCurveDefinition> sumCasePairs = dlg.summaryAddressSelection()->selectedCurveDefinitions();
            if (sumCasePairs.size() == 1)
            {
                m_case = sumCasePairs[0].summaryCase();
                m_summaryAddress->setAddress(sumCasePairs[0].summaryAddress());
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimCalculationVariable::summaryAddressDisplayString() const
{
    return "test";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCalculationVariable::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_name);
    uiOrdering.add(&m_summaryAddressUi);
    uiOrdering.add(&m_button);

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCalculationVariable::defineObjectEditorAttribute(QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    caf::PdmUiTableViewEditorAttribute* attr = dynamic_cast<caf::PdmUiTableViewEditorAttribute*>(attribute);
    if (attr)
    {
        attr->registerPushButtonTextForFieldKeyword(m_button.keyword(), "Edit");
    }
}
