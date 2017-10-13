/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicSummaryCurveCalculator.h"

#include "RiaApplication.h"

#include "RimCalculation.h"
#include "RimCalculationCollection.h"
#include "RimProject.h"

#include "cafPdmUiListEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"


CAF_PDM_SOURCE_INIT(RicSummaryCurveCalculator, "RicSummaryCurveCalculator");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSummaryCurveCalculator::RicSummaryCurveCalculator()
{
    CAF_PDM_InitObject("RicSummaryCurveCalculator", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_currentCalculation, "CurrentCalculation", "Current Calculation", "", "", "");
    m_currentCalculation.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);
    //m_currentCalculation.uiCapability()->setUiEditorTypeName(caf::PdmUiTreeSelectionEditor::uiEditorTypeName());
    m_currentCalculation.uiCapability()->setUiEditorTypeName(caf::PdmUiListEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_newCalculation, "NewCalculation", "New Calculation", "", "", "");
    RicSummaryCurveCalculator::assignPushButtonEditor(&m_newCalculation);
    
    CAF_PDM_InitFieldNoDefault(&m_deleteCalculation, "DeleteCalculation", "Delete Calculation", "", "", "");
    RicSummaryCurveCalculator::assignPushButtonEditor(&m_deleteCalculation);

    CAF_PDM_InitFieldNoDefault(&m_parseExpression, "ParseExpression", "Parse", "", "", "");
    RicSummaryCurveCalculator::assignPushButtonEditor(&m_parseExpression);

    CAF_PDM_InitFieldNoDefault(&m_calculateExpression, "CalculateExpression", "Calculate", "", "", "");
    RicSummaryCurveCalculator::assignPushButtonEditor(&m_calculateExpression);

    CAF_PDM_InitFieldNoDefault(&m_newVariable, "NewVariable", "New Variable", "", "", "");
    RicSummaryCurveCalculator::assignPushButtonEditor(&m_newVariable);

    CAF_PDM_InitFieldNoDefault(&m_deleteVariable, "DeleteVariable", "Delete Variable", "", "", "");
    RicSummaryCurveCalculator::assignPushButtonEditor(&m_deleteVariable);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicSummaryCurveCalculator::calculatedSummariesGroupName()
{
    return "CalculatedSummariesGroupName";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicSummaryCurveCalculator::calulationGroupName()
{
    return "CalulationGroupName";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCalculation* RicSummaryCurveCalculator::currentCalculation() const
{
    return m_currentCalculation();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCalculator::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_newCalculation)
    {
        m_newCalculation = false;

        RimCalculation* rimCalc = calculationCollection()->addCalculation();
        m_currentCalculation = rimCalc;

        this->updateConnectedEditors();
    }
    else if (changedField == &m_deleteCalculation)
    {
        m_deleteCalculation = false;

        if (m_currentCalculation())
        {
            calculationCollection()->deleteCalculation(m_currentCalculation());
            m_currentCalculation = nullptr;

            this->updateConnectedEditors();
        }
    }
    else if (changedField == &m_parseExpression)
    {
        m_parseExpression = false;

        if (m_currentCalculation())
        {
            m_currentCalculation()->parseExpression();

            this->updateConnectedEditors();
        }
    }
    else if (changedField == &m_calculateExpression)
    {
        m_calculateExpression = false;

        if (m_currentCalculation())
        {
            m_currentCalculation()->calculate();
        }
    }
    else if (changedField == &m_newVariable)
    {
        m_newVariable = false;

        if (m_currentCalculation())
        {
            m_currentCalculation()->addVariable();

            this->updateConnectedEditors();
        }
    }
    else if (changedField == &m_deleteVariable)
    {
        m_deleteVariable = false;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCalculator::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    if (!m_currentCalculation())
    {
        if (calculationCollection()->calculations().size() > 0)
        {
            m_currentCalculation = calculationCollection()->calculations()[0];
        }
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroupWithKeyword("Calculated Summaries", RicSummaryCurveCalculator::calculatedSummariesGroupName());
        group->add(&m_currentCalculation);
        group->add(&m_newCalculation);
        group->add(&m_deleteCalculation);
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroupWithKeyword("Calculation Settings", RicSummaryCurveCalculator::calulationGroupName());
        if (m_currentCalculation())
        {
            m_currentCalculation->uiOrdering(uiConfigName, *group);
        }

        group->add(&m_parseExpression);
        group->add(&m_calculateExpression);

//         group->add(&m_newVariable);
//         group->add(&m_deleteVariable);
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RicSummaryCurveCalculator::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;
    if (fieldNeedingOptions == &m_currentCalculation)
    {
        for (auto c : calculationCollection()->calculations())
        {
            options.push_back(caf::PdmOptionItemInfo(c->description(), c));
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCalculationCollection* RicSummaryCurveCalculator::calculationCollection()
{
    RimProject* proj = RiaApplication::instance()->project();
    if (proj)
    {
        return proj->calculationCollection();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCalculator::assignPushButtonEditor(caf::PdmFieldHandle* fieldHandle)
{
    CVF_ASSERT(fieldHandle);
    CVF_ASSERT(fieldHandle->uiCapability());

    fieldHandle->uiCapability()->setUiEditorTypeName(caf::PdmUiPushButtonEditor::uiEditorTypeName());
    fieldHandle->uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCalculator::assignPushButtonEditorText(caf::PdmUiEditorAttribute* attribute, const QString& text)
{
    caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*> (attribute);
    if (attrib)
    {
        attrib->m_buttonText = text;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCalculator::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (&m_newCalculation == field)
    {
        RicSummaryCurveCalculator::assignPushButtonEditorText(attribute, "New Calculation");
    }
    else if (&m_deleteCalculation == field)
    {
        RicSummaryCurveCalculator::assignPushButtonEditorText(attribute, "Delete Calculation");
    }
    else if (&m_newVariable == field)
    {
        RicSummaryCurveCalculator::assignPushButtonEditorText(attribute, "New Variable");
    }
    else if (&m_deleteVariable == field)
    {
        RicSummaryCurveCalculator::assignPushButtonEditorText(attribute, "Delete Variable");
    }
    else if (&m_parseExpression == field)
    {
        RicSummaryCurveCalculator::assignPushButtonEditorText(attribute, "Parse Expression");
    }
    else if (&m_calculateExpression == field)
    {
        RicSummaryCurveCalculator::assignPushButtonEditorText(attribute, "Calculate");
    }
}
