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

#include "RimSummaryCalculationVariable.h"

#include "RiaSummaryCurveDefinition.h"

#include "RimSummaryAddress.h"
#include "RimSummaryCalculation.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"

#include "RiuSummaryCurveDefSelection.h"
#include "RiuSummaryCurveDefSelectionDialog.h"

#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTableView.h"


CAF_PDM_SOURCE_INIT(RimSummaryCalculationVariable, "RimSummaryCalculationVariable");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCalculationVariable::RimSummaryCalculationVariable()
{
    CAF_PDM_InitObject("RimSummaryCalculationVariable", ":/octave.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_name,             "VariableName",     "Variable Name", "", "", "");
    m_name.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitFieldNoDefault(&m_button, "PushButton", "", "", "", "");
    m_button.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_button.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault(&m_summaryAddressUi,   "SummaryAddressUi",   "Summary Address", "", "", "");
    m_summaryAddressUi.registerGetMethod(this, &RimSummaryCalculationVariable::summaryAddressDisplayString);
    m_summaryAddressUi.xmlCapability()->disableIO();
    m_summaryAddressUi.uiCapability()->setUiReadOnly(true);

    
    CAF_PDM_InitFieldNoDefault(&m_case, "SummaryCase", "Summary Case", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_summaryAddress, "SummaryAddress", "Summary Address", "", "", "");
    m_summaryAddress = new RimSummaryAddress;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimSummaryCalculationVariable::name() const
{
    return m_name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationVariable::setName(const QString& name)
{
    m_name = name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationVariable::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_button)
    {
        bool updateContainingEditor = false;

        {
            RiuSummaryCurveDefSelectionDialog dlg(nullptr);
            {
                std::vector<RiaSummaryCurveDefinition> sumCasePairs;
                sumCasePairs.push_back(RiaSummaryCurveDefinition(m_case(), m_summaryAddress->address()));

                dlg.summaryAddressSelection()->setSelectedCurveDefinitions(sumCasePairs);
                dlg.summaryAddressSelection()->updateConnectedEditors();
                dlg.updateLabel();
            }

            if (dlg.exec() == QDialog::Accepted)
            {
                std::vector<RiaSummaryCurveDefinition> sumCasePairs = dlg.summaryAddressSelection()->selectedCurveDefinitions();
                if (sumCasePairs.size() == 1)
                {
                    m_case = sumCasePairs[0].summaryCase();
                    m_summaryAddress->setAddress(sumCasePairs[0].summaryAddress());

                    updateContainingEditor = true;
                }
            }
        }

        if (updateContainingEditor)
        {
            RimSummaryCalculation* rimCalculation = nullptr;
            this->firstAncestorOrThisOfTypeAsserted(rimCalculation);

            // RimCalculation is pointed to by RicSummaryCurveCalculator in a PtrField
            // Update editors connected to RicSummaryCurveCalculator
            std::vector<caf::PdmObjectHandle*> referringObjects;
            rimCalculation->objectsWithReferringPtrFields(referringObjects);
            for (auto o : referringObjects)
            {
                o->uiCapability()->updateConnectedEditors();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimSummaryCalculationVariable::summaryAddressDisplayString() const
{
    return RiaSummaryCurveDefinition::curveDefinitionText(m_case(), m_summaryAddress()->address());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimSummaryCalculationVariable::summaryCase()
{
    return m_case();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryAddress* RimSummaryCalculationVariable::summaryAddress()
{
    return m_summaryAddress();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationVariable::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_name);
    uiOrdering.add(&m_summaryAddressUi);
    uiOrdering.add(&m_button);

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationVariable::defineObjectEditorAttribute(QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    caf::PdmUiTableViewEditorAttribute* attr = dynamic_cast<caf::PdmUiTableViewEditorAttribute*>(attribute);
    if (attr)
    {
        attr->registerPushButtonTextForFieldKeyword(m_button.keyword(), "Edit");
    }
}
