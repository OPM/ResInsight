/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
#include "RimGridCrossPlotCurveSet.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RigActiveCellInfo.h"
#include "RigActiveCellsResultAccessor.h"
#include "RigCaseCellResultCalculator.h"
#include "RigEclipseCrossPlotDataExtractor.h"

#include "RigFormationNames.h"
#include "RigMainGrid.h"

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimEclipseResultDefinition.h"
#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotCurve.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimTools.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiSliderEditor.h"

#include <QString>

CAF_PDM_SOURCE_INIT(RimGridCrossPlotCurveSet, "GridCrossPlotCurveSet");

namespace caf
{
template<>
void RimGridCrossPlotCurveSet::CurveCategorizationEnum::setUp()
{
    addItem(RigGridCrossPlotCurveCategorization::NO_CATEGORIZATION, "NONE", "None");
    addItem(RigGridCrossPlotCurveCategorization::TIME_CATEGORIZATION, "TIME", "Time");
    addItem(RigGridCrossPlotCurveCategorization::FORMATION_CATEGORIZATION, "FORMATION", "Formations");
    addItem(RigGridCrossPlotCurveCategorization::RESULT_CATEGORIZATION, "RESULT", "Result Property");
    setDefault(RigGridCrossPlotCurveCategorization::TIME_CATEGORIZATION);
}
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCrossPlotCurveSet::RimGridCrossPlotCurveSet()
{
    CAF_PDM_InitObject("Cross Plot Data Set", ":/WellLogCurve16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_case, "Case", "Case", "", "", "");
    m_case.uiCapability()->setUiTreeChildrenHidden(true);
    CAF_PDM_InitField(&m_timeStep, "TimeStep", -1, "Time Step", "", "", "");
    m_timeStep.uiCapability()->setUiEditorTypeName(caf::PdmUiComboBoxEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_cellFilterView, "VisibleCellView", "Limit to Cells Visible in View", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_categorization, "Categorization", "Data Categorization", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_xAxisProperty, "XAxisProperty", "X-Axis Property", "", "", "");
    m_xAxisProperty = new RimEclipseResultDefinition;
    m_xAxisProperty.uiCapability()->setUiHidden(true);
    m_xAxisProperty.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_yAxisProperty, "YAxisProperty", "Y-Axis Property", "", "", "");
    m_yAxisProperty = new RimEclipseResultDefinition;
    m_yAxisProperty.uiCapability()->setUiHidden(true);
    m_yAxisProperty.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_categoryProperty, "CategoryProperty", "Categorisation Property", "", "", "");
    m_categoryProperty = new RimEclipseResultDefinition;
    m_categoryProperty.uiCapability()->setUiHidden(true);
    m_categoryProperty.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitField(&m_categoryBinCount, "CategoryBinCount", 2, "Category Bin Count", "", "", "");
    m_categoryBinCount.uiCapability()->setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_nameConfig, "NameConfig", "Name", "", "", "");
    m_nameConfig = new RimGridCrossPlotCurveSetNameConfig(this);
    m_nameConfig.uiCapability()->setUiTreeHidden(true);
    m_nameConfig.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_crossPlotCurves, "CrossPlotCurves", "Curves", "", "", "");
    m_crossPlotCurves.uiCapability()->setUiTreeHidden(true);

    setDefaults();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::loadDataAndUpdate(bool updateParentPlot)
{
    onLoadDataAndUpdate(updateParentPlot);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::setParentQwtPlotNoReplot(QwtPlot* parent)
{
    for (auto& curve : m_crossPlotCurves())
    {
        curve->setParentQwtPlotNoReplot(m_isChecked() ? parent : nullptr);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotCurveSet::xAxisName() const
{
    return m_xAxisProperty->resultVariableUiShortName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotCurveSet::yAxisName() const
{
    return m_yAxisProperty->resultVariableUiShortName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimGridCrossPlotCurveSet::indexInPlot() const
{
    RimGridCrossPlot* parent;
    this->firstAncestorOrThisOfTypeAsserted(parent);
    return parent->indexOfCurveSet(this);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotCurveSet::createAutoName() const
{
    if (m_case() == nullptr)
    {
        return "Undefined";
    }

    QStringList nameTags;
    if (!m_nameConfig->customName().isEmpty())
    {
        nameTags += m_nameConfig->customName();
    }

    if (m_nameConfig->addCaseName())
    {
        nameTags += m_case->caseUserDescription();
    }

    if (m_nameConfig->addAxisVariables())
    {
        nameTags += QString("%1 x %2").arg(xAxisName(), yAxisName());
    }

    if (m_nameConfig->addTimestep())
    {
        if (m_xAxisProperty->hasDynamicResult() || m_yAxisProperty->hasDynamicResult())
        {
            if (m_timeStep() == -1)
            {
                nameTags += "All Time Steps";
            }
            else
            {
                QStringList timeStepNames = m_case->timeStepStrings();
                nameTags += timeStepNames[m_timeStep()];
            }
        }
    }

    if (m_nameConfig->addCategorization)
    {
        if (m_categorization() == FORMATION_CATEGORIZATION)
        {
            nameTags += QString("Categorized by formations");
        }
        else if (m_categorization() == RESULT_CATEGORIZATION && m_categoryProperty->hasResult())
        {
            nameTags += QString("Categorized by %1").arg(m_categoryProperty->resultVariableUiShortName());
        }
    }

    return nameTags.join(", ");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotCurveSet::createShortAutoName() const
{
    if (m_case() == nullptr)
    {
        return "Undefined";
    }

    QStringList nameTags;
    if (!m_nameConfig->customName().isEmpty())
    {
        nameTags += m_nameConfig->customName();
    }

    if (m_nameConfig->addCaseName())
    {
        nameTags += m_case->caseUserDescription();
    }

    if (m_nameConfig->addAxisVariables())
    {
        nameTags += QString("%1 x %2").arg(xAxisName(), yAxisName());
    }
    return nameTags.join(", ");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::detachAllCurves()
{
    for (auto curve : m_crossPlotCurves())
    {
        curve->detachQwtCurve();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::cellFilterViewUpdated()
{
    if (m_cellFilterView())
    {
        loadDataAndUpdate(true);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGridCrossPlotCurve*> RimGridCrossPlotCurveSet::curves() const
{
    return m_crossPlotCurves.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::initAfterRead()
{
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case());
    if (eclipseCase)
    {
        m_xAxisProperty->setEclipseCase(eclipseCase);
        m_yAxisProperty->setEclipseCase(eclipseCase);
        m_categoryProperty->setEclipseCase(eclipseCase);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::onLoadDataAndUpdate(bool updateParentPlot)
{
    performAutoNameUpdate();
    
    detachAllCurves();
    m_crossPlotCurves.deleteAllChildObjects();

    if (m_case() == nullptr)
    {
        return;
    }
        
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());

    if (eclipseCase == nullptr)
    {
        return;
    }

    if (!eclipseCase->ensureReservoirCaseIsOpen())
    {
        RiaLogging::warning(QString("Failed to open eclipse grid file %1").arg(eclipseCase->gridFileName()));

        return;
    }

    RigEclipseResultAddress xAddress(m_xAxisProperty->resultType(), m_xAxisProperty->resultVariable());
    RigEclipseResultAddress yAddress(m_yAxisProperty->resultType(), m_yAxisProperty->resultVariable());
    RigEclipseResultAddress catAddress(m_categoryProperty->resultType(), m_categoryProperty->resultVariable());

    std::map<int, cvf::UByteArray> timeStepCellVisibilityMap = calculateCellVisibility(eclipseCase);

    RigEclipseCrossPlotResult result = RigEclipseCrossPlotDataExtractor::extract(
        eclipseCase->eclipseCaseData(), m_timeStep(), xAddress, yAddress, m_categorization(), catAddress, m_categoryBinCount, timeStepCellVisibilityMap);

    for (const auto& sampleCategory : result.categorySamplesMap)
    {
        RimGridCrossPlotCurve* curve = new RimGridCrossPlotCurve();
        
        QString categoryName = result.categoryNameMap[sampleCategory.first];

        if (categoryName.isEmpty())
        {
            curve->setCustomName(createShortAutoName());
        }
        else
        {
            curve->setCustomName(QString("%1 : %2").arg(createShortAutoName()).arg(categoryName));
        }
        curve->determineColorAndSymbol(indexInPlot(), sampleCategory.first, (int)result.categorySamplesMap.size(), m_categorization() == FORMATION_CATEGORIZATION);
        curve->setSamples(sampleCategory.second.first, sampleCategory.second.second);
        curve->updateCurveAppearance();
        curve->updateCurveNameAndUpdatePlotLegendAndTitle();
        curve->updateUiIconFromPlotSymbol();

        m_crossPlotCurves.push_back(curve);
    }
    
    if (updateParentPlot)
    {
        triggerReplotAndTreeRebuild();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<int, cvf::UByteArray> RimGridCrossPlotCurveSet::calculateCellVisibility(RimEclipseCase* eclipseCase) const
{
    std::map<int, cvf::UByteArray> timeStepCellVisibilityMap;
    if (m_cellFilterView)
    {
        RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(m_cellFilterView());
        if (eclipseView)
        {
            std::set<int> timeSteps;
            if (m_timeStep() == -1)
            {
                for (int i = 0; i < (int)eclipseCase->timeStepDates().size(); ++i)
                {
                    timeSteps.insert(i);
                }
            }
            else
            {
                timeSteps.insert(m_timeStep());
            }
            for (int i : timeSteps)
            {
                eclipseView->calculateCurrentTotalCellVisibility(&timeStepCellVisibilityMap[i], i);
            }
        }
    }
    return timeStepCellVisibilityMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_case);
    if (m_case)
    {
        uiOrdering.add(&m_timeStep);
        uiOrdering.add(&m_cellFilterView);
        uiOrdering.add(&m_categorization);

        if (m_categorization() == RESULT_CATEGORIZATION)
        {
            caf::PdmUiGroup* categoryGroup = uiOrdering.addNewGroup("Categorization Property");
            m_categoryProperty->uiOrdering(uiConfigName, *categoryGroup);
            categoryGroup->add(&m_categoryBinCount);
        }

        caf::PdmUiGroup* xAxisGroup = uiOrdering.addNewGroup("X-Axis Property");
        m_xAxisProperty->uiOrdering(uiConfigName, *xAxisGroup);

        caf::PdmUiGroup* yAxisGroup = uiOrdering.addNewGroup("Y-Axis Property");
        m_yAxisProperty->uiOrdering(uiConfigName, *yAxisGroup);
    }

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup("Name Configuration");
    m_nameConfig->uiOrdering(uiConfigName, *nameGroup);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                                const QVariant&            oldValue,
                                                const QVariant&            newValue)
{
    if (changedField == &m_case)
    {
        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());
        if (eclipseCase)
        {
            m_xAxisProperty->setEclipseCase(eclipseCase);
            m_yAxisProperty->setEclipseCase(eclipseCase);
            m_categoryProperty->setEclipseCase(eclipseCase);
            // TODO: Do we need all these??
            m_xAxisProperty->updateConnectedEditors();
            m_yAxisProperty->updateConnectedEditors();
            m_categoryProperty->updateConnectedEditors();

            loadDataAndUpdate(true);
        }
    }
    else if (changedField == &m_timeStep)
    {
        loadDataAndUpdate(true);
    }
    else if (changedField == &m_categorization || changedField == &m_categoryBinCount)
    {
        loadDataAndUpdate(true);
    }
    else if (changedField == &m_cellFilterView)
    {
        loadDataAndUpdate(true);
    }
    else if (changedField == &m_isChecked)
    {
        triggerReplotAndTreeRebuild();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimGridCrossPlotCurveSet::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                              bool*                      useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_case)
    {
        RimTools::eclipseCaseOptionItems(&options);
        if (options.empty())
        {
            options.push_front(caf::PdmOptionItemInfo("None", nullptr));
        }
    }
    else if (fieldNeedingOptions == &m_timeStep)
    {
        QStringList timeStepNames;

        if (m_case)
        {
            timeStepNames = m_case->timeStepStrings();
        }
        options.push_back(caf::PdmOptionItemInfo("All Time Steps", -1));
        for (int i = 0; i < timeStepNames.size(); i++)
        {
            options.push_back(caf::PdmOptionItemInfo(timeStepNames[i], i));
        }
    }
    else if (fieldNeedingOptions == &m_cellFilterView)
    {
        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case());
        if (eclipseCase)
        {
            options.push_back(caf::PdmOptionItemInfo("Disabled", nullptr));
            for (RimEclipseView* view : eclipseCase->reservoirViews.childObjects())
            {
                options.push_back(caf::PdmOptionItemInfo(view->name(), view, false, view->uiIcon()));
            }
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::triggerReplotAndTreeRebuild()
{
    RimGridCrossPlot* parent;
    this->firstAncestorOrThisOfTypeAsserted(parent);
    parent->reattachCurvesToQwtAndReplot();
    parent->updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::performAutoNameUpdate()
{
    this->setName(createAutoName());
    RimGridCrossPlot* parent;
    this->firstAncestorOrThisOfTypeAsserted(parent);
    parent->performAutoNameUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::setDefaults()
{
    RimProject* project = RiaApplication::instance()->project();
    if (project)
    {
        if (!project->eclipseCases().empty())
        {
            RimEclipseCase* eclipseCase = project->eclipseCases().front();
            m_case = eclipseCase;
            m_xAxisProperty->setEclipseCase(eclipseCase);
            m_yAxisProperty->setEclipseCase(eclipseCase);
            m_categoryProperty->setEclipseCase(eclipseCase);

            m_xAxisProperty->setResultType(RiaDefines::DYNAMIC_NATIVE);
            m_xAxisProperty->setResultVariable("SOIL");

            m_yAxisProperty->setResultType(RiaDefines::DYNAMIC_NATIVE);
            m_yAxisProperty->setResultVariable("PRESSURE");
        }
    }    
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::defineEditorAttribute(const caf::PdmFieldHandle* field,
                                                     QString                    uiConfigName,
                                                     caf::PdmUiEditorAttribute* attribute)
{
    if (field == &m_categoryBinCount)
    {
        auto myAttr = dynamic_cast<caf::PdmUiSliderEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_minimum = 2;
            myAttr->m_maximum = 50;
        }
    }
}

CAF_PDM_SOURCE_INIT(RimGridCrossPlotCurveSetNameConfig, "RimGridCrossPlotCurveSetNameConfig");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridCrossPlotCurveSetNameConfig::RimGridCrossPlotCurveSetNameConfig(RimNameConfigHolderInterface* parent)
    : RimNameConfig(parent)
{
    CAF_PDM_InitObject("Cross Plot Curve Set NameGenerator", "", "", "");

    CAF_PDM_InitField(&addCaseName, "AddCaseName", false, "Add Case Name", "", "", "");
    CAF_PDM_InitField(&addAxisVariables, "AddAxisVariables", true, "Add Axis Variables", "", "", "");
    CAF_PDM_InitField(&addTimestep, "AddTimeStep", false, "Add Time Step", "", "", "");
    CAF_PDM_InitField(&addCategorization, "AddCategorization", false, "Add Data Categorization", "", "", "");

    setCustomName("");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSetNameConfig::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&addCaseName);
    uiOrdering.add(&addAxisVariables);
    uiOrdering.add(&addTimestep);
    uiOrdering.add(&addCategorization);
}
