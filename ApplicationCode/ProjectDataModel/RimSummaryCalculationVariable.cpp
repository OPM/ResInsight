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

#include "RiaApplication.h"
#include "RiaSummaryCurveDefinition.h"

#include "RifEclipseSummaryAddressQMetaType.h"

#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCalculation.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"

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

    CAF_PDM_InitFieldNoDefault(&m_button,           "PushButton", "", "", "", "");
    m_button.uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    m_button.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault(&m_summaryAddressUi, "SummaryAddressUi",   "Summary Address", "", "", "");
    m_summaryAddressUi.registerGetMethod(this, &RimSummaryCalculationVariable::summaryAddressDisplayString);
    m_summaryAddressUi.xmlCapability()->disableIO();
    m_summaryAddressUi.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitFieldNoDefault(&m_case,             "SummaryCase",      "Summary Case", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_summaryAddress,   "SummaryAddress",   "Summary Address", "", "", "");

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
            dlg.hideEnsembles();

            readDataFromApplicationStore(&dlg);

            if (dlg.exec() == QDialog::Accepted)
            {
                std::vector<RiaSummaryCurveDefinition> curveSelection = dlg.curveSelection();
                if (curveSelection.size() > 0)
                {
                    m_case = curveSelection[0].summaryCase();
                    m_summaryAddress->setAddress(curveSelection[0].summaryAddress());

                    writeDataToApplicationStore();

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
    QString caseName;
    if (m_case() ) caseName = m_case()->caseName();

    return RiaSummaryCurveDefinition::curveDefinitionText(caseName, m_summaryAddress()->address());
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationVariable::readDataFromApplicationStore(RiuSummaryCurveDefSelectionDialog* selectionDialog) const
{
    if (!selectionDialog) return;

    auto sumCase    = m_case();
    auto sumAddress = m_summaryAddress->address();
    if (!sumCase && !sumAddress.isValid())
    {
        QVariant var = RiaApplication::instance()->cacheDataObject("CalculatorSummaryAddress");

        auto lastUsedAddress = var.value<RifEclipseSummaryAddress>();
        if (lastUsedAddress.isValid())
        {
            sumAddress = lastUsedAddress;
        }
        
        QString lastUsedSummaryCaseString = RiaApplication::instance()->cacheDataObject("CalculatorSummaryCase").toString();

        auto* lastUsedSummaryCase = dynamic_cast<RimSummaryCase*>(
            caf::PdmReferenceHelper::objectFromReference(RiaApplication::instance()->project(), lastUsedSummaryCaseString));
        if (lastUsedSummaryCase)
        {
            sumCase = lastUsedSummaryCase;
        }
    }

    selectionDialog->setCaseAndAddress(sumCase, sumAddress);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculationVariable::writeDataToApplicationStore() const
{
    QString refFromProjectToObject =
        caf::PdmReferenceHelper::referenceFromRootToObject(RiaApplication::instance()->project(), m_case);
    RiaApplication::instance()->setCacheDataObject("CalculatorSummaryCase", refFromProjectToObject);

    QVariant sumAdrVar = QVariant::fromValue(m_summaryAddress->address());
    RiaApplication::instance()->setCacheDataObject("CalculatorSummaryAddress", sumAdrVar);
}
