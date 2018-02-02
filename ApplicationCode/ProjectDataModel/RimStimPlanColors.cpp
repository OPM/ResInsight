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

#include "RiaApplication.h"
#include "RiaFractureDefines.h"

#include "RimEclipseView.h"
#include "RimFractureTemplateCollection.h"
#include "RimLegendConfig.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimStimPlanFractureTemplate.h"

#include "cafPdmUiItem.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafSelectionManagerTools.h"

#include "cvfqtUtils.h"

#include <cmath> // Needed for HUGE_VAL on Linux



CAF_PDM_SOURCE_INIT(RimStimPlanColors, "RimStimPlanColors");

namespace caf {

    template<>
    void caf::AppEnum< RimStimPlanColors::StimPlanResultColorType >::setUp()
    {
        addItem(RimStimPlanColors::COLOR_INTERPOLATION, "COLOR_INTERPOLATION", "On");
        addItem(RimStimPlanColors::SINGLE_ELEMENT_COLOR, "SINGLE_ELEMENT_COLOR", "Off");

        setDefault(RimStimPlanColors::COLOR_INTERPOLATION);
    }

} // End namespace caf

//--------------------------------------------------------------------------------------------------
/// Internal methods
//--------------------------------------------------------------------------------------------------
static void setDefaultResultIfStimPlan(caf::PdmField<QString> &field);
static QString toString(const std::pair<QString, QString>& resultNameAndUnit);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimStimPlanColors::RimStimPlanColors()
{
    CAF_PDM_InitObject("StimPlan Colors", ":/FractureSymbol16x16.png", "", "");

    CAF_PDM_InitField(&m_resultNameAndUnit, "ResultName", QString(""),  "Result Variable", "", "", "");

    CAF_PDM_InitField(&m_defaultColor,      "DefaultColor", cvf::Color3f(cvf::Color3::BROWN), "Default Color", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_legendConfigurations, "LegendConfigurations", "", "", "", "");
    m_legendConfigurations.uiCapability()->setUiTreeHidden(true);

    CAF_PDM_InitField(&m_showStimPlanMesh, "ShowStimPlanMesh", true, "Show Mesh", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_stimPlanCellVizMode, "StimPlanCellVizMode", "Color Interpolation", "", "", "");

    setName("Fracture");
    nameField()->uiCapability()->setUiReadOnly(true);
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
void RimStimPlanColors::setShowStimPlanMesh(bool showStimPlanMesh)
{
    m_showStimPlanMesh = showStimPlanMesh;
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
                if (toString(resultNameAndUnit) == legendVariableName)
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
        QString resultNameUnitString = toString(resultNameAndUnit);
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

        options.push_back(caf::PdmOptionItemInfo("None", ""));

        for (auto resultNameAndUnit : fractureTemplates->stimPlanResultNamesAndUnits())
        {
            QString resultNameAndUnitString = toString(resultNameAndUnit);
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
        updateStimPlanTemplates();
    }

    if (changedField == &m_showStimPlanMesh)
    {
        RimProject* proj;
        this->firstAncestorOrThisOfType(proj);
        if (proj)
        {
            proj->createDisplayModelAndRedrawAllViews();
        }
    }

    if(changedField == &m_stimPlanCellVizMode)
    {
        Rim3dView* rimView = nullptr;
        this->firstAncestorOrThisOfType(rimView);
        if (rimView)
        {
            rimView->createDisplayModelAndRedraw();
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
QString RimStimPlanColors::uiResultName() const
{
    return RimStimPlanColors::toResultName(m_resultNameAndUnit());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanColors::setDefaultResultNameForStimPlan()
{
    setDefaultResultIfStimPlan(m_resultNameAndUnit);
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
        double posClosestToZero = HUGE_VAL;
        double negClosestToZero = -HUGE_VAL;

        RimFractureTemplateCollection* fracTemplateColl = fractureTemplateCollection();

        fracTemplateColl->computeMinMax(uiResultName(), unit(), &minValue, &maxValue, &posClosestToZero, &negClosestToZero);

        if (minValue != HUGE_VAL)
        {
            legendConfig->setAutomaticRanges(minValue, maxValue, minValue, maxValue);
            legendConfig->setClosestToZeroValues(posClosestToZero, negClosestToZero, posClosestToZero, negClosestToZero);
        }

        legendConfig->setTitle(m_resultNameAndUnit());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanColors::updateStimPlanTemplates() const
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
            stimPlanFracTemplate->updateFractureGrid();
        }
        proj->createDisplayModelAndRedrawAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimStimPlanColors::updateConductivityResultName()
{
    // Conductivity result name as reported from the XML file was moved  to the fracture template
    // Replace all conductivity names with the one and only conductivity uiName reported from by fracture template

    if (m_resultNameAndUnit().contains("conductivity", Qt::CaseInsensitive))
    {
        RimFractureTemplateCollection* fractureTemplates = fractureTemplateCollection();
        for (auto resultNameAndUnit : fractureTemplates->stimPlanResultNamesAndUnits())
        {
            if (resultNameAndUnit.first.contains("conductivity", Qt::CaseInsensitive))
            {
                QString resultNameAndUnitString = toString(resultNameAndUnit);
                m_resultNameAndUnit = resultNameAndUnitString;
            }
        }
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
    caf::PdmUiGroup* colorGroup = uiOrdering.addNewGroup("Colors");
    colorGroup->add(&m_resultNameAndUnit);
    colorGroup->add(&m_defaultColor);

    bool stimPlanExists = false;
    RimProject* proj;
    this->firstAncestorOrThisOfType(proj);
    std::vector<RimFractureTemplate*> fracTemplates = proj->allFractureTemplates();

    for (auto fractemplate : fracTemplates)
    {
        if (dynamic_cast<RimStimPlanFractureTemplate*>(fractemplate))
        {
            stimPlanExists = true;
            break;
        }
    }

    if (stimPlanExists)
    {
        caf::PdmUiGroup* stimPlanGroup = uiOrdering.addNewGroup("StimPlan");
        stimPlanGroup->add(&m_showStimPlanMesh);
        stimPlanGroup->add(&m_stimPlanCellVizMode);
    }

    uiOrdering.skipRemainingFields(true);
}


//--------------------------------------------------------------------------------------------------
/// Internal methods
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString toString(const std::pair<QString, QString>& resultNameAndUnit)
{
    return QString("%1 [%2]").arg(resultNameAndUnit.first).arg(resultNameAndUnit.second);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void setDefaultResultIfStimPlan(caf::PdmField<QString> &field)
{
    RimProject* proj = RiaApplication::instance()->project();

    std::vector<RimFractureTemplate*> stimPlanFracTemplates = proj->allFractureTemplates();

    QString currentResultName = field();

    if (!stimPlanFracTemplates.empty() && (currentResultName.isEmpty() || currentResultName == "None"))
    {
        RimFractureTemplateCollection* templColl = proj->allFractureTemplateCollections().front();

        for (auto resultNameAndUnit : templColl->stimPlanResultNamesAndUnits())
        {
            if (resultNameAndUnit.first.contains(RiaDefines::conductivityResultName(), Qt::CaseInsensitive))
            {
                field = toString(resultNameAndUnit);
                break;
            }
        }
    }

}
