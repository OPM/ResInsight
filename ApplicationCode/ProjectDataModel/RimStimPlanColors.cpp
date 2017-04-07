/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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

#include "RimStimPlanColors.h"

#include "RimEclipseView.h"
#include "RimFractureTemplateCollection.h"
#include "RimLegendConfig.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimStimPlanFractureTemplate.h"

#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiItem.h"
#include "cafPdmUiTreeOrdering.h"

#include "cvfqtUtils.h"

#include <cmath> // Needed for HUGE_VAL on Linux



CAF_PDM_SOURCE_INIT(RimStimPlanColors, "RimStimPlanColors");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimStimPlanColors::RimStimPlanColors()
{
    CAF_PDM_InitObject("StimPlan Colors", ":/FractureSymbol16x16.png", "", "");

    CAF_PDM_InitField(&m_resultNameAndUnit, "ResultName", QString(""),  "StimPlan Result Variable", "", "", "");
    CAF_PDM_InitField(&m_opacityLevel,      "opacityLevel", 1.0f,       "Transparency", "", "", "");
    m_opacityLevel.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_defaultColor,      "DefaultColor", cvf::Color3f(cvf::Color3::BROWN), "Default Color", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_legendConfigurations, "LegendConfigurations", "", "", "", "");
    m_legendConfigurations.uiCapability()->setUiTreeHidden(true);

    m_name = "Fracture Colors";
    m_name.uiCapability()->setUiReadOnly(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimStimPlanColors::~RimStimPlanColors()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanColors::loadDataAndUpdate()
{
    RimFractureTemplateCollection* fractureTemplates = fractureTemplateCollection();

    std::vector<std::pair<QString, QString> > resultNameAndUnits = fractureTemplates->stimPlanResultNamesAndUnits();

    // Delete legends referencing results not present on file
    {
        std::vector<RimLegendConfig*> toBeDeleted;
        for (RimLegendConfig* legend : m_legendConfigurations)
        {
            QString legendVariableName = legend->resultVariableName();

            bool found = false;
            for (auto resultNameAndUnit : resultNameAndUnits)
            {
                if (RimStimPlanColors::toString(resultNameAndUnit) == legendVariableName)
                {
                    found = true;
                }
            }

            if (!found)
            {
                toBeDeleted.push_back(legend);
            }
        }

        for (auto legend : toBeDeleted)
        {
            m_legendConfigurations.removeChildObject(legend);

            delete legend;
        }
    }

    // Create legend for result if not already present
    for (auto resultNameAndUnit : resultNameAndUnits)
    {
        QString resultNameUnitString = RimStimPlanColors::toString(resultNameAndUnit);
        bool foundResult = false;

        for (RimLegendConfig* legend : m_legendConfigurations)
        {
            if (legend->resultVariableName() == resultNameUnitString)
            {
                foundResult = true;
            }
        }

        if (!foundResult)
        {
            RimLegendConfig* legendConfig = new RimLegendConfig();
            legendConfig->resultVariableName = resultNameUnitString;
            legendConfig->setMappingMode(RimLegendConfig::LINEAR_DISCRETE);
            legendConfig->setColorRangeMode(RimLegendConfig::STIMPLAN);


            m_legendConfigurations.push_back(legendConfig);
        }
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimStimPlanColors::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_resultNameAndUnit)
    {
        RimFractureTemplateCollection* fractureTemplates = fractureTemplateCollection();

        options.push_back(caf::PdmOptionItemInfo("None", "None"));

        for (auto resultNameAndUnit : fractureTemplates->stimPlanResultNamesAndUnits())
        {
            QString resultNameAndUnitString = RimStimPlanColors::toString(resultNameAndUnit);
            options.push_back(caf::PdmOptionItemInfo(resultNameAndUnitString, resultNameAndUnitString));
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanColors::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimEclipseView* sourceView = nullptr;
    this->firstAncestorOrThisOfType(sourceView);
    if (sourceView)
    {
        sourceView->scheduleCreateDisplayModelAndRedraw();
    }

    if (changedField == &m_resultNameAndUnit)
    {
        //Get all frac templates and re-generate stimplan cells
        RimProject* proj;
        this->firstAncestorOrThisOfType(proj);
        if (proj)
        {
            std::vector<RimStimPlanFractureTemplate*> stimPlanFracTemplates;
            proj->descendantsIncludingThisOfType(stimPlanFracTemplates);
            for (RimStimPlanFractureTemplate* stimPlanFracTemplate : stimPlanFracTemplates)
            {
                stimPlanFracTemplate->setupStimPlanCells();
            }
            proj->createDisplayModelAndRedrawAllViews();
        }



    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimLegendConfig* RimStimPlanColors::activeLegend() const
{
    for (RimLegendConfig* legendConfig : m_legendConfigurations)
    {
        if (m_resultNameAndUnit == legendConfig->resultVariableName())
        {
            return legendConfig;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimStimPlanColors::resultName() const
{
    return RimStimPlanColors::toResultName(m_resultNameAndUnit());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimStimPlanColors::unit() const
{
    return RimStimPlanColors::toUnit(m_resultNameAndUnit());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float RimStimPlanColors::opacityLevel() const
{
    return m_opacityLevel();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimStimPlanColors::defaultColor() const
{
    return m_defaultColor();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanColors::updateLegendData()
{
    RimLegendConfig* legendConfig = activeLegend();
    if (legendConfig)
    {
        double minValue = HUGE_VAL;
        double maxValue = -HUGE_VAL;

        RimFractureTemplateCollection* fracTemplateColl = fractureTemplateCollection();

        fracTemplateColl->computeMinMax(resultName(), unit(), &minValue, &maxValue);

        if (minValue != HUGE_VAL)
        {
            legendConfig->setAutomaticRanges(minValue, maxValue, minValue, maxValue);
        }

        legendConfig->setTitle(cvfqt::Utils::toString(m_resultNameAndUnit()));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureTemplateCollection* RimStimPlanColors::fractureTemplateCollection() const
{
    RimProject* proj = nullptr;
    this->firstAncestorOrThisOfType(proj);

    return proj->activeOilField()->fractureDefinitionCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimStimPlanColors::toString(const std::pair<QString, QString>& resultNameAndUnit)
{
    return QString("%1 [%2]").arg(resultNameAndUnit.first).arg(resultNameAndUnit.second);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimStimPlanColors::toResultName(const QString& resultNameAndUnit)
{
    QStringList items = resultNameAndUnit.split("[");

    if (items.size() > 0)
    {
        return items[0].trimmed();
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimStimPlanColors::toUnit(const QString& resultNameAndUnit)
{
    int start = resultNameAndUnit.indexOf("[");
    int end = resultNameAndUnit.indexOf("]");

    if (start != -1 && end != -1)
    {
        return resultNameAndUnit.mid(start + 1, end - start - 1);
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanColors::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    if (activeLegend())
    {
        uiTreeOrdering.add(activeLegend());
    }

    uiTreeOrdering.skipRemainingChildren(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanColors::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_resultNameAndUnit);
    uiOrdering.add(&m_defaultColor);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanColors::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (field == &m_opacityLevel)
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_minimum = 0.0;
            myAttr->m_maximum = 1.0;
            myAttr->m_decimals = 2;
            myAttr->m_sliderTickCount = 100;
        }
    }
}

