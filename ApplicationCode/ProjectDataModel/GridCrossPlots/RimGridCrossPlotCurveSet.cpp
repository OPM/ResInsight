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
#include "RiaColorTables.h"
#include "RiaLogging.h"

#include "RigActiveCellInfo.h"
#include "RigActiveCellsResultAccessor.h"
#include "RigCaseCellResultCalculator.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseCrossPlotDataExtractor.h"

#include "RigFormationNames.h"
#include "RigMainGrid.h"

#include "RiuGridCrossQwtPlot.h"

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultDefinition.h"
#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotCurve.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"
#include "RimTools.h"

#include "cafColorTable.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiSliderEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cvfScalarMapper.h"

#include <QString>

CAF_PDM_SOURCE_INIT(RimGridCrossPlotCurveSet, "GridCrossPlotCurveSet");

namespace caf
{
template<>
void RimGridCrossPlotCurveSet::CurveCategorizationEnum::setUp()
{
    addItem(RigGridCrossPlotCurveCategorization::NO_CATEGORIZATION, "NONE", "Nothing");
    addItem(RigGridCrossPlotCurveCategorization::TIME_CATEGORIZATION, "TIME", "Time Step");
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

    CAF_PDM_InitFieldNoDefault(&m_cellFilterView, "VisibleCellView", "Filter by Cells Visible in 3d View", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_categorization, "Categorization", "Group Data by", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_xAxisProperty, "XAxisProperty", "X-Axis Property", "", "", "");
    m_xAxisProperty = new RimEclipseResultDefinition;
    m_xAxisProperty.uiCapability()->setUiHidden(true);
    m_xAxisProperty.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_yAxisProperty, "YAxisProperty", "Y-Axis Property", "", "", "");
    m_yAxisProperty = new RimEclipseResultDefinition;
    m_yAxisProperty.uiCapability()->setUiHidden(true);
    m_yAxisProperty.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_categoryProperty, "CategoryProperty", "Data Grouping Property", "", "", "");
    m_categoryProperty = new RimEclipseCellColors;
    m_categoryProperty.uiCapability()->setUiHidden(true);

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
void RimGridCrossPlotCurveSet::setCellFilterView(RimGridView* cellFilterView)
{
    m_cellFilterView = cellFilterView;
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
        nameTags += caseNameString();
    }

    if (m_nameConfig->addAxisVariables())
    {
        nameTags += axisVariableString();
    }

    if (m_nameConfig->addTimestep() && !timeStepString().isEmpty())
    {
        nameTags += timeStepString();        
    }

    if (m_nameConfig->addCategorization)
    {
        QString catTitle = categoryTitle();
        if (!catTitle.isEmpty()) nameTags += catTitle;
    }

    return nameTags.join(", ");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotCurveSet::categoryTitle() const
{
    if (m_categorization() == TIME_CATEGORIZATION)
    {
        return QString("Time Steps");
    }
    else if (m_categorization() == FORMATION_CATEGORIZATION)
    {
        return QString("Formations");
    }
    else if (m_categorization() == RESULT_CATEGORIZATION && m_categoryProperty->hasResult())
    {
        return QString("%1").arg(m_categoryProperty->resultVariableUiShortName());
    }
    return "";
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
RimRegularLegendConfig* RimGridCrossPlotCurveSet::legendConfig() const
{
    return m_categoryProperty->legendConfig();
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
QString RimGridCrossPlotCurveSet::caseNameString() const
{
    if (m_case())
    {
        return m_case->caseUserDescription();
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotCurveSet::axisVariableString() const
{
    return QString("%1 x %2").arg(xAxisName(), yAxisName());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotCurveSet::timeStepString() const
{
    // If using time categorization, the time step will be included as a category, so skip it here.
    if (m_categorization() != RigGridCrossPlotCurveCategorization::TIME_CATEGORIZATION)
    {
        if (m_case() && (m_xAxisProperty->hasDynamicResult() || m_yAxisProperty->hasDynamicResult()))
        {
            if (m_timeStep == -1)
            {
                return "All Time Steps";
            }
            return m_case->timeStepStrings()[m_timeStep];
        }
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimGridCrossPlotCurveSet::categoryStrings() const
{
    std::vector<QString> catStrings;
    for (auto curve : m_crossPlotCurves())
    {
        catStrings.push_back(legendConfig()->categoryNameFromCategoryValue(curve->categoryIndex()));
    }
    return catStrings;
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
    updateDataSetName();
    
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

    updateLegend();

    RigEclipseCrossPlotResult result = RigEclipseCrossPlotDataExtractor::extract(
        eclipseCase->eclipseCaseData(), m_timeStep(), xAddress, yAddress, m_categorization(), catAddress, timeStepCellVisibilityMap);

    createCurves(result);

    if (updateParentPlot)
    {
        triggerPlotNameUpdateAndReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::createCurves(const RigEclipseCrossPlotResult& result)
{
    if (m_categorization == NO_CATEGORIZATION)
    {
        const caf::ColorTable& colors     = RiaColorTables::contrastCategoryPaletteColors();
        int                    colorIndex = indexInPlot();

        RimGridCrossPlotCurve* curve = new RimGridCrossPlotCurve();
        curve->setColor(colors.cycledColor3f(colorIndex));
        curve->setCategoryInformation(indexInPlot(), 0);
        curve->setSamples(result.xValues, result.yValues);
        curve->updateCurveAppearance();
        curve->updateUiIconFromPlotSymbol();
        m_crossPlotCurves.push_back(curve);
    }
    else
    {
        std::map<int, std::pair<std::vector<double>, std::vector<double>>> categorizedResults;

        std::vector<double> tickValues;

        if (hasCategoryResult())
        {
            for (size_t i = 0; i < result.xValues.size(); ++i)
            {
                int categoryNum =
                    m_categorization == RESULT_CATEGORIZATION
                    ? static_cast<int>(result.catValuesContinuous[i])
                    : result.catValuesDiscrete[i];

                categorizedResults[categoryNum].first.push_back(result.xValues[i]);
                categorizedResults[categoryNum].second.push_back(result.yValues[i]);
            }
        }
        else
        {
            legendConfig()->scalarMapper()->majorTickValues(&tickValues);

            for (size_t i = 0; i < result.xValues.size(); ++i)
            {
                auto upperBoundIt = std::lower_bound(tickValues.begin(), tickValues.end(), result.catValuesContinuous[i]);
                int  upperBoundIndex = static_cast<int>(upperBoundIt - tickValues.begin());
                int  lowerBoundIndex = std::min((int) tickValues.size() - 2, std::max(0, upperBoundIndex - 1));
                categorizedResults[lowerBoundIndex].first.push_back(result.xValues[i]);
                categorizedResults[lowerBoundIndex].second.push_back(result.yValues[i]);
            }
        }

        for (auto it = categorizedResults.rbegin(); it != categorizedResults.rend(); ++it)
        {
            RimGridCrossPlotCurve* curve = new RimGridCrossPlotCurve();
            curve->setCategoryInformation(indexInPlot(), it->first);
            if (hasCategoryResult())
            {
                curve->setColor(cvf::Color3f(legendConfig()->scalarMapper()->mapToColor(it->first)));                
            }
            else
            {
                curve->setColor(cvf::Color3f(legendConfig()->scalarMapper()->mapToColor(tickValues[it->first])));
            }
            curve->setSamples(it->second.first, it->second.second);
            curve->showLegend(m_crossPlotCurves.empty());
            curve->setLegendEntryTitle(createAutoName());
            curve->updateCurveAppearance();
            curve->updateUiIconFromPlotSymbol();
            m_crossPlotCurves.push_back(curve);
        }
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

        if (m_categorization() == TIME_CATEGORIZATION &&
            !(m_xAxisProperty->hasDynamicResult() || m_yAxisProperty->hasDynamicResult()))
        {
            m_categorization = NO_CATEGORIZATION;
        }

        if (m_categorization() == RESULT_CATEGORIZATION)
        {
            caf::PdmUiGroup* categoryGroup = uiOrdering.addNewGroup("Data Grouping Property");
            m_categoryProperty->uiOrdering(uiConfigName, *categoryGroup);
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
    else if (changedField == &m_categorization)
    {
        if (m_categorization == TIME_CATEGORIZATION)
        {
            legendConfig()->setColorRange(RimRegularLegendConfig::NORMAL);
            legendConfig()->setMappingMode(RimRegularLegendConfig::CATEGORY_INTEGER);
        }
        else if (hasCategoryResult())
        {
            legendConfig()->setColorRange(RimRegularLegendConfig::CATEGORY);
            legendConfig()->setMappingMode(RimRegularLegendConfig::CATEGORY_INTEGER);
        }
        else
        {
            legendConfig()->setColorRange(RimRegularLegendConfig::NORMAL);
            legendConfig()->setMappingMode(RimRegularLegendConfig::LINEAR_DISCRETE);

        }
       
        loadDataAndUpdate(true);
    }
    else if (changedField == &m_cellFilterView)
    {
        loadDataAndUpdate(true);
    }
    else if (changedField == &m_isChecked)
    {
        updateLegend();
        triggerPlotNameUpdateAndReplot();
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
    else if (fieldNeedingOptions == &m_categorization)
    {
        std::set<RigGridCrossPlotCurveCategorization> validOptions = { NO_CATEGORIZATION, TIME_CATEGORIZATION, FORMATION_CATEGORIZATION, RESULT_CATEGORIZATION };
        if (m_timeStep() != -1 || !(m_xAxisProperty->hasDynamicResult() || m_yAxisProperty->hasDynamicResult()))
        {
            validOptions.erase(TIME_CATEGORIZATION);
        }
        for (auto optionItem : validOptions)
        {
            options.push_back(caf::PdmOptionItemInfo(CurveCategorizationEnum::uiText(optionItem), optionItem));
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::updateLegend()
{
    legendConfig()->setTitle(categoryTitle());

    RimGridCrossPlot* parent;
    this->firstAncestorOrThisOfTypeAsserted(parent);
    if (parent->qwtPlot())
    {
        if (m_categorization() != NO_CATEGORIZATION && m_case() && isChecked() && legendConfig()->showLegend())
        {
            if (m_categorization() == FORMATION_CATEGORIZATION)
            {
                RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case());
                RigFormationNames* formationNames = eclipseCase->eclipseCaseData()->activeFormationNames();

                const std::vector<QString>& categoryNames = formationNames->formationNames();
                legendConfig()->setNamedCategories(categoryNames);
                legendConfig()->setAutomaticRanges(0, categoryNames.size() - 1, 0, categoryNames.size() - 1);
            }
            else if (m_categorization() == TIME_CATEGORIZATION)                
            {
                QStringList timeStepNames = m_case->timeStepStrings();
                std::vector<QString> categoryNames;
                for (auto name : timeStepNames)
                {
                    categoryNames.push_back(name);
                }
                legendConfig()->setNamedCategories(categoryNames);
                legendConfig()->setAutomaticRanges(0, categoryNames.size() - 1, 0, categoryNames.size() - 1);
            }
            else if (m_categoryProperty->eclipseResultAddress().isValid())
            {
                RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case());
                if (eclipseCase)
                {
                    m_categoryProperty->updateLegendData(eclipseCase, m_timeStep());
                }
            }
            parent->qwtPlot()->addOrUpdateCurveSetLegend(this);
        }
        else
        {
            parent->qwtPlot()->removeCurveSetLegend(this);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCrossPlotCurveSet::hasCategoryResult() const
{
    if (m_categorization == FORMATION_CATEGORIZATION || m_categorization == TIME_CATEGORIZATION)
    {
        return true;
    }
    else if (m_categorization == RESULT_CATEGORIZATION)
    {
        return m_categoryProperty->hasCategoryResult();
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::triggerPlotNameUpdateAndReplot()
{
    RimGridCrossPlot* parent;
    this->firstAncestorOrThisOfTypeAsserted(parent);
    parent->updateCurveNamesAndPlotTitle();
    parent->reattachCurvesToQwtAndReplot();
    parent->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::updateCurveNames(bool applyCaseName,
                                                bool applyAxisVariables,
                                                bool applyTimeStep,
                                                bool applyCategory)
{
    for (auto curve : m_crossPlotCurves())
    {
        QStringList nameTags;

        if (applyCaseName)
        {
            nameTags += caseNameString();
        }

        if (applyAxisVariables)
        {
            nameTags += axisVariableString();
        }

        if (applyTimeStep && !timeStepString().isEmpty())
        {
            nameTags += timeStepString();
        }

        if (applyCategory && m_categorization != NO_CATEGORIZATION)
        {
            if (hasCategoryResult())
            {
                nameTags += legendConfig()->categoryNameFromCategoryValue(curve->categoryIndex());
            }
            else
            {
                std::vector<double> tickValues;
                legendConfig()->scalarMapper()->majorTickValues(&tickValues);
                size_t catIndex = (size_t) curve->categoryIndex();
                double lowerLimit = tickValues[catIndex];
                double upperLimit = catIndex + 1u < tickValues.size()
                    ? tickValues[catIndex + 1u] : std::numeric_limits<double>::infinity();
                nameTags += QString("%1 [%2, %3]").arg(categoryTitle()).arg(lowerLimit).arg(upperLimit);
            }
        }

        curve->setCustomName(nameTags.join(", "));
        if (m_categorization != NO_CATEGORIZATION)
        {
            curve->setLegendEntryTitle(createAutoName());
        }

        curve->updateCurveNameAndUpdatePlotLegendAndTitle();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::updateDataSetName()
{
    this->setName(createAutoName());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::performAutoNameUpdate()
{
    updateDataSetName();
    triggerPlotNameUpdateAndReplot();
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    if (m_categorization() != NO_CATEGORIZATION)
    {
        m_categoryProperty->uiTreeOrdering(uiTreeOrdering, uiConfigName);
    }

    for (auto curve : m_crossPlotCurves())
    {
        uiTreeOrdering.add(curve);
    }

    uiTreeOrdering.skipRemainingChildren(true);
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
    CAF_PDM_InitField(&addCategorization, "AddCategorization", true, "Add Data Categorization", "", "", "");

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
