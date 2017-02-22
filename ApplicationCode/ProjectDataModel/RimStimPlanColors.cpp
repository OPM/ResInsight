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
    CAF_PDM_InitObject("StimPlan Colors", ":/draw_style_faults_24x24.png", "", "");

    CAF_PDM_InitField(&m_resultNameAndUnit, "ResultName", QString(""),  "Result Variable", "", "", "");
    CAF_PDM_InitField(&m_opacityLevel,      "opacityLevel", 0.2f,       "Transparency", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_legendConfigurations, "LegendConfigurations", "", "", "", "");
    m_legendConfigurations.uiCapability()->setUiTreeHidden(true);

    m_name = "StimPlan Colors";
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

    uiTreeOrdering.setForgetRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanColors::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_resultNameAndUnit);
    uiOrdering.add(&m_opacityLevel);
    uiOrdering.setForgetRemainingFields(true);
}

