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
#include "RiaSummaryTools.h"

#include "RimProject.h"
#include "RimSummaryCalculation.h"
#include "RimSummaryCalculationCollection.h"

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
RimSummaryCalculation* RicSummaryCurveCalculator::currentCalculation() const
{
    return m_currentCalculation();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSummaryCurveCalculator::parseExpression() const
{
    if (m_currentCalculation())
    {
        QString previousCurveName = m_currentCalculation->description();
        if (!m_currentCalculation()->parseExpression())
        {
            return false;
        }

        QString currentCurveName = m_currentCalculation->description();
        if (previousCurveName != currentCurveName)
        {
            RiaSummaryTools::notifyCalculatedCurveNameHasChanged(previousCurveName, currentCurveName);
        }

        m_currentCalculation()->updateDependentCurvesAndPlots();
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryCurveCalculator::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_newCalculation)
    {
        m_newCalculation = false;

        RimSummaryCalculation* rimCalc = calculationCollection()->addCalculation();
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
RimSummaryCalculationCollection* RicSummaryCurveCalculator::calculationCollection()
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
bool RicSummaryCurveCalculator::calculate() const
{
    if (m_currentCalculation())
    {
        QString previousCurveName = m_currentCalculation->description();
        if (!m_currentCalculation()->parseExpression())
        {
            return false;
        }

        QString currentCurveName = m_currentCalculation->description();
        if (previousCurveName != currentCurveName)
        {
            RiaSummaryTools::notifyCalculatedCurveNameHasChanged(previousCurveName, currentCurveName);
        }

        if (!m_currentCalculation()->calculate())
        {
            return false;
        }

        m_currentCalculation()->updateDependentCurvesAndPlots();
    }

    return true;
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
}
