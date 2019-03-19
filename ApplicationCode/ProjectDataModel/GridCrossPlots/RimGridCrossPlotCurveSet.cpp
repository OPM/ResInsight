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

#include "RifEclipseDataTableFormatter.h"

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
#include "RimEclipseCellColors.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotCurve.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"
#include "RimTools.h"

#include "CellFilters/RimPlotCellFilterCollection.h"

#include "cafCategoryMapper.h"
#include "cafColorTable.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiSliderEditor.h"
#include "cafPdmUiTreeOrdering.h"
#include "cafProgressInfo.h"
#include "cvfScalarMapper.h"
#include "cvfqtUtils.h"

#include <QString>

CAF_PDM_SOURCE_INIT(RimGridCrossPlotCurveSet, "GridCrossPlotCurveSet");

namespace caf
{
template<>
void RimGridCrossPlotCurveSet::CurveGroupingEnum::setUp()
{
    addItem(RigGridCrossPlotCurveGrouping::NO_GROUPING, "NONE", "Nothing");
    addItem(RigGridCrossPlotCurveGrouping::GROUP_BY_TIME, "TIME", "Time Step");
    addItem(RigGridCrossPlotCurveGrouping::GROUP_BY_FORMATION, "FORMATION", "Formations");
    addItem(RigGridCrossPlotCurveGrouping::GROUP_BY_RESULT, "RESULT", "Result Property");
    setDefault(RigGridCrossPlotCurveGrouping::GROUP_BY_TIME);
}
} // namespace caf

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

    CAF_PDM_InitFieldNoDefault(&m_cellFilterView, "VisibleCellView", "Filter by 3d View Visibility", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_grouping, "Grouping", "Group Data by", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_xAxisProperty, "XAxisProperty", "X-Axis Property", "", "", "");
    m_xAxisProperty = new RimEclipseResultDefinition;
    m_xAxisProperty.uiCapability()->setUiHidden(true);
    m_xAxisProperty.uiCapability()->setUiTreeChildrenHidden(true);
    m_xAxisProperty->setLabelsOnTop(true);

    CAF_PDM_InitFieldNoDefault(&m_yAxisProperty, "YAxisProperty", "Y-Axis Property", "", "", "");
    m_yAxisProperty = new RimEclipseResultDefinition;
    m_yAxisProperty.uiCapability()->setUiHidden(true);
    m_yAxisProperty.uiCapability()->setUiTreeChildrenHidden(true);
    m_yAxisProperty->setLabelsOnTop(true);

    CAF_PDM_InitFieldNoDefault(&m_groupingProperty, "GroupingProperty", "Data Grouping Property", "", "", "");
    m_groupingProperty = new RimEclipseCellColors;
    m_groupingProperty.uiCapability()->setUiHidden(true);
    m_groupingProperty->legendConfig()->setMappingMode(RimRegularLegendConfig::CATEGORY_INTEGER);

    CAF_PDM_InitFieldNoDefault(&m_nameConfig, "NameConfig", "Name", "", "", "");
    m_nameConfig = new RimGridCrossPlotCurveSetNameConfig(this);
    m_nameConfig.uiCapability()->setUiTreeHidden(true);
    m_nameConfig.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_crossPlotCurves, "CrossPlotCurves", "Curves", "", "", "");
    m_crossPlotCurves.uiCapability()->setUiTreeHidden(true);

    CAF_PDM_InitField(&m_useCustomColor, "UseCustomColor", false, "Use Custom Color", "", "", "");
    CAF_PDM_InitField(&m_customColor, "CustomColor", cvf::Color3f(cvf::Color3f::BLACK), "Custom Color", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_plotCellFilterCollection, "PlotCellFilterCollection", "Cell Filters", "", "", "");
    m_plotCellFilterCollection = new RimPlotCellFilterCollection;

    setDefaults();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::setCellFilterView(RimGridView* cellFilterView)
{
    m_cellFilterView = cellFilterView;
    m_groupingProperty->setReservoirView(dynamic_cast<RimEclipseView*>(m_cellFilterView()));
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

    if (m_nameConfig->addGrouping() && groupParameter() != "None")
    {
        QString catTitle = groupTitle();
        if (!catTitle.isEmpty()) nameTags += catTitle;
    }

    return nameTags.join(", ");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotCurveSet::groupTitle() const
{
    if (m_grouping != NO_GROUPING)
    {
        return QString("Grouped by %1").arg(groupParameter());
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotCurveSet::groupParameter() const
{
    if (m_grouping() == GROUP_BY_TIME)
    {
        return QString("Time Steps");
    }
    else if (m_grouping() == GROUP_BY_FORMATION)
    {
        return QString("Formations");
    }
    else if (m_grouping() == GROUP_BY_RESULT && m_groupingProperty->hasResult())
    {
        return QString("%1").arg(m_groupingProperty->resultVariableUiShortName());
    }
    return "None";
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
    return m_groupingProperty->legendConfig();
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
    if (m_grouping() != RigGridCrossPlotCurveGrouping::GROUP_BY_TIME)
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
std::vector<QString> RimGridCrossPlotCurveSet::groupStrings() const
{
    std::vector<QString> groupStrings;
    for (auto curve : m_crossPlotCurves())
    {
        groupStrings.push_back(legendConfig()->categoryNameFromCategoryValue(curve->groupIndex()));
    }
    return groupStrings;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<RimGridCrossPlotCurveSet::NameComponents, QString> RimGridCrossPlotCurveSet::nameComponents() const
{
    std::map<RimGridCrossPlotCurveSet::NameComponents, QString> componentNames;
    if (m_nameConfig->addCaseName()) componentNames[GCP_CASE_NAME] = caseNameString();
    if (m_nameConfig->addAxisVariables()) componentNames[GCP_AXIS_VARIABLES] = axisVariableString();
    if (m_nameConfig->addTimestep()) componentNames[GCP_TIME_STEP] = timeStepString();
    if (m_nameConfig->addGrouping()) componentNames[GCP_GROUP_NAME] = groupTitle();

    return componentNames;
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
        m_groupingProperty->setEclipseCase(eclipseCase);
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
    RigEclipseResultAddress groupAddress(m_groupingProperty->resultType(), m_groupingProperty->resultVariable());

    std::map<int, cvf::UByteArray> timeStepCellVisibilityMap = calculateCellVisibility(eclipseCase);

    updateLegend();

    RigEclipseCrossPlotResult result = RigEclipseCrossPlotDataExtractor::extract(
        eclipseCase->eclipseCaseData(), m_timeStep(), xAddress, yAddress, m_grouping(), groupAddress, timeStepCellVisibilityMap);

    if (isXAxisLogarithmic() || isYAxisLogarithmic())
    {
        filterInvalidCurveValues(&result);
    }
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
    m_groupedResults.clear();
    if (!groupingEnabled())
    {
        RimGridCrossPlotCurve* curve = new RimGridCrossPlotCurve();
        if (m_useCustomColor)
        {
            curve->setColor(m_customColor());
        }
        else
        {
            const caf::ColorTable& colors     = RiaColorTables::contrastCategoryPaletteColors();
            int                    colorIndex = indexInPlot();
            curve->setColor(colors.cycledColor3f(colorIndex));
        }
        curve->setGroupingInformation(indexInPlot(), 0);
        curve->setSamples(result.xValues, result.yValues);
        curve->updateCurveAppearance();
        curve->updateUiIconFromPlotSymbol();
        m_crossPlotCurves.push_back(curve);
        m_groupedResults[0] = result;
    }
    else
    {
        std::vector<double> tickValues;

        if (groupingByCategoryResult())
        {
            for (size_t i = 0; i < result.xValues.size(); ++i)
            {
                int categoryNum = m_grouping == GROUP_BY_RESULT ? static_cast<int>(result.groupValuesContinuous[i])
                                                                : result.groupValuesDiscrete[i];

                m_groupedResults[categoryNum].xValues.push_back(result.xValues[i]);
                m_groupedResults[categoryNum].yValues.push_back(result.yValues[i]);
                if (!result.groupValuesContinuous.empty())
                    m_groupedResults[categoryNum].groupValuesContinuous.push_back(result.groupValuesContinuous[i]);
                if (!result.groupValuesDiscrete.empty())
                    m_groupedResults[categoryNum].groupValuesDiscrete.push_back(result.groupValuesDiscrete[i]);
            }
        }
        else
        {
            legendConfig()->scalarMapper()->majorTickValues(&tickValues);

            for (size_t i = 0; i < result.xValues.size(); ++i)
            {
                auto upperBoundIt    = std::lower_bound(tickValues.begin(), tickValues.end(), result.groupValuesContinuous[i]);
                int  upperBoundIndex = static_cast<int>(upperBoundIt - tickValues.begin());
                int  categoryNum     = std::min((int)tickValues.size() - 2, std::max(0, upperBoundIndex - 1));
                m_groupedResults[categoryNum].xValues.push_back(result.xValues[i]);
                m_groupedResults[categoryNum].yValues.push_back(result.yValues[i]);
                if (!result.groupValuesContinuous.empty())
                    m_groupedResults[categoryNum].groupValuesContinuous.push_back(result.groupValuesContinuous[i]);
                if (!result.groupValuesDiscrete.empty())
                    m_groupedResults[categoryNum].groupValuesDiscrete.push_back(result.groupValuesDiscrete[i]);
            }
        }

        for (auto it = m_groupedResults.rbegin(); it != m_groupedResults.rend(); ++it)
        {
            RimGridCrossPlotCurve* curve = new RimGridCrossPlotCurve();
            curve->setGroupingInformation(indexInPlot(), it->first);
            if (groupingByCategoryResult())
            {
                curve->setColor(cvf::Color3f(legendConfig()->scalarMapper()->mapToColor(it->first)));
            }
            else
            {
                curve->setColor(cvf::Color3f(legendConfig()->scalarMapper()->mapToColor(tickValues[it->first])));
            }
            curve->setSamples(it->second.xValues, it->second.yValues);
            curve->showLegend(m_crossPlotCurves.empty());
            curve->setLegendEntryText(createAutoName());
            curve->updateCurveAppearance();
            curve->updateUiIconFromPlotSymbol();
            m_crossPlotCurves.push_back(curve);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGridCrossPlotCurveSet::createGroupName(size_t groupIndex) const
{
    if (groupingByCategoryResult())
    {
        return legendConfig()->categoryNameFromCategoryValue(groupIndex);
    }
    else
    {
        std::vector<double> tickValues;
        legendConfig()->scalarMapper()->majorTickValues(&tickValues);
        double lowerLimit = tickValues[groupIndex];
        double upperLimit =
            groupIndex + 1u < tickValues.size() ? tickValues[groupIndex + 1u] : std::numeric_limits<double>::infinity();
        return QString("%1 [%2, %3]").arg(groupParameter()).arg(lowerLimit).arg(upperLimit);
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
    else if (m_plotCellFilterCollection->cellFilterCount() > 0)
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

        RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();
        if (eclipseCaseData)
        {
            RiaDefines::PorosityModelType porosityModel = RiaDefines::MATRIX_MODEL;

            RigCaseCellResultsData* cellResultsData = eclipseCaseData->results(porosityModel);
            if (cellResultsData)
            {
                const RigActiveCellInfo* actCellInfo = cellResultsData->activeCellInfo();
                size_t                   cellCount   = actCellInfo->reservoirCellCount();

                for (int i : timeSteps)
                {
                    cvf::UByteArray* cellVisibility = &timeStepCellVisibilityMap[i];
                    cellVisibility->resize(cellCount);
                    cellVisibility->setAll(true);

                    m_plotCellFilterCollection->computeCellVisibilityFromFilter(i, cellVisibility);
                }
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
        uiOrdering.add(&m_grouping);

        if (m_grouping() == GROUP_BY_TIME && !(m_xAxisProperty->hasDynamicResult() || m_yAxisProperty->hasDynamicResult()))
        {
            m_grouping = NO_GROUPING;
        }

        if (m_grouping() == GROUP_BY_RESULT)
        {
            caf::PdmUiGroup* dataGroupingGroup = uiOrdering.addNewGroup("Data Grouping Property");
            m_groupingProperty->uiOrdering(uiConfigName, *dataGroupingGroup);
        }

        caf::PdmUiGroup* xAxisGroup = uiOrdering.addNewGroup("X-Axis Property");
        m_xAxisProperty->uiOrdering(uiConfigName, *xAxisGroup);

        caf::PdmUiGroup* yAxisGroup = uiOrdering.addNewGroup("Y-Axis Property", false);
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
            m_groupingProperty->setEclipseCase(eclipseCase);
            // TODO: Do we need all these??
            m_xAxisProperty->updateConnectedEditors();
            m_yAxisProperty->updateConnectedEditors();
            m_groupingProperty->updateConnectedEditors();

            loadDataAndUpdate(true);
        }
    }
    else if (changedField == &m_timeStep)
    {
        if (m_timeStep != -1 && m_grouping == GROUP_BY_TIME)
        {
            m_grouping = NO_GROUPING;
        }

        loadDataAndUpdate(true);
    }
    else if (changedField == &m_grouping)
    {
        if (m_grouping == GROUP_BY_TIME)
        {
            legendConfig()->setColorRange(RimRegularLegendConfig::NORMAL);
            legendConfig()->setMappingMode(RimRegularLegendConfig::CATEGORY_INTEGER);
        }
        else if (groupingByCategoryResult())
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
        m_groupingProperty->setReservoirView(dynamic_cast<RimEclipseView*>(m_cellFilterView()));
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
    else if (fieldNeedingOptions == &m_grouping)
    {
        std::set<RigGridCrossPlotCurveGrouping> validOptions = {NO_GROUPING, GROUP_BY_TIME, GROUP_BY_FORMATION, GROUP_BY_RESULT};
        if (!hasMultipleTimeSteps())
        {
            validOptions.erase(GROUP_BY_TIME);
        }
        {
            RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case());
            if (!eclipseCase || !eclipseCase->eclipseCaseData()->activeFormationNames())
            {
                validOptions.erase(GROUP_BY_FORMATION);
            }
        }
        for (auto optionItem : validOptions)
        {
            options.push_back(caf::PdmOptionItemInfo(CurveGroupingEnum::uiText(optionItem), optionItem));
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::updateLegend()
{
    legendConfig()->setTitle(groupParameter());
    legendConfig()->disableAllTimeStepsRange(!hasMultipleTimeSteps());

    RimGridCrossPlot* parent;
    this->firstAncestorOrThisOfTypeAsserted(parent);
    if (parent->qwtPlot())
    {
        if (groupingEnabled() && m_case() && isChecked() && legendConfig()->showLegend())
        {
            if (m_grouping() == GROUP_BY_FORMATION)
            {
                RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case());
                if (eclipseCase)
                {
                    RigFormationNames* formationNames = eclipseCase->eclipseCaseData()->activeFormationNames();
                    if (formationNames)
                    {
                        const std::vector<QString>& categoryNames = formationNames->formationNames();
                        if (!categoryNames.empty())
                        {
                            legendConfig()->setNamedCategories(categoryNames);
                            legendConfig()->setAutomaticRanges(0, categoryNames.size() - 1, 0, categoryNames.size() - 1);
                        }
                    }
                }
            }
            else if (m_grouping() == GROUP_BY_TIME)
            {
                QStringList          timeStepNames = m_case->timeStepStrings();
                std::vector<QString> categoryNames;
                for (auto name : timeStepNames)
                {
                    categoryNames.push_back(name);
                }
                if (!categoryNames.empty())
                {
                    legendConfig()->setNamedCategories(categoryNames);
                    legendConfig()->setAutomaticRanges(0, categoryNames.size() - 1, 0, categoryNames.size() - 1);
                }
            }
            else if (m_groupingProperty->eclipseResultAddress().isValid())
            {
                RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case());
                if (eclipseCase)
                {
                    m_groupingProperty->updateLegendData(eclipseCase, m_timeStep());
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
bool RimGridCrossPlotCurveSet::groupingByCategoryResult() const
{
    if (m_grouping == GROUP_BY_FORMATION || m_grouping == GROUP_BY_TIME)
    {
        return true;
    }
    else if (m_grouping == GROUP_BY_RESULT)
    {
        return m_groupingProperty->hasCategoryResult();
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCrossPlotCurveSet::groupingEnabled() const
{
    if (m_grouping != NO_GROUPING)
    {
        if (m_grouping == GROUP_BY_RESULT && !m_groupingProperty->eclipseResultAddress().isValid())
        {
            return false;
        }
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::swapAxisProperties(bool updatePlot)
{
    RimEclipseResultDefinition* xAxisProperties = m_xAxisProperty();
    RimEclipseResultDefinition* yAxisProperties = m_yAxisProperty();

    m_xAxisProperty.removeChildObject(xAxisProperties);
    m_yAxisProperty.removeChildObject(yAxisProperties);
    m_yAxisProperty = xAxisProperties;
    m_xAxisProperty = yAxisProperties;

    updateConnectedEditors();
    loadDataAndUpdate(updatePlot);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::exportFormattedData(RifEclipseDataTableFormatter& formatter) const
{
    if (m_groupedResults.empty()) return;

    if (m_grouping != NO_GROUPING)
    {
        std::vector<RifEclipseOutputTableColumn> header = {RifEclipseOutputTableColumn("X"),
                                                           RifEclipseOutputTableColumn("Y"),
                                                           RifEclipseOutputTableColumn("Group Index"),
                                                           RifEclipseOutputTableColumn("Group Description")};

        formatter.header(header);
    }
    else
    {
        std::vector<RifEclipseOutputTableColumn> header = {RifEclipseOutputTableColumn("X"), RifEclipseOutputTableColumn("Y")};
        formatter.header(header);
    }

    caf::ProgressInfo progress(m_groupedResults.size(), "Gathering Data Points");
    for (auto it = m_groupedResults.begin(); it != m_groupedResults.end(); ++it)
    {
        auto task = progress.task(QString("Exporting Group %1").arg(it->first));

        RigEclipseCrossPlotResult res = it->second;

        for (size_t i = 0; i < it->second.xValues.size(); ++i)
        {
            if (m_grouping() == NO_GROUPING)
            {
                formatter.add(res.xValues[i]);
                formatter.add(res.yValues[i]);
            }
            else
            {
                int     groupIndex = it->first;
                QString groupName  = createGroupName(groupIndex);
                formatter.add(res.xValues[i]);
                formatter.add(res.yValues[i]);
                formatter.add(groupIndex);
                formatter.add(groupName);
            }
            formatter.rowCompleted();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCrossPlotCurveSet::isXAxisLogarithmic() const
{
    RimGridCrossPlot* parent = nullptr;
    firstAncestorOrThisOfTypeAsserted(parent);
    return parent->isXAxisLogarithmic();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCrossPlotCurveSet::isYAxisLogarithmic() const
{
    RimGridCrossPlot* parent = nullptr;
    firstAncestorOrThisOfTypeAsserted(parent);
    return parent->isYAxisLogarithmic();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::setFromCaseAndEquilibriumRegion(RimEclipseResultCase* eclipseCase,
                                                               const QString&        dynamicResultName)
{
    m_case = eclipseCase;

    m_xAxisProperty->setEclipseCase(eclipseCase);
    m_xAxisProperty->setResultType(RiaDefines::DYNAMIC_NATIVE);
    m_xAxisProperty->setResultVariable(dynamicResultName);

    m_yAxisProperty->setEclipseCase(eclipseCase);
    m_yAxisProperty->setResultType(RiaDefines::STATIC_NATIVE);
    m_yAxisProperty->setResultVariable("DEPTH");

    m_grouping = NO_GROUPING;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::addCellFilter(RimPlotCellFilter* cellFilter)
{
    m_plotCellFilterCollection->addCellFilter(cellFilter);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::setCustomColor(const cvf::Color3f color)
{
    m_useCustomColor = true;
    m_customColor    = color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::triggerPlotNameUpdateAndReplot()
{
    RimGridCrossPlot* parent;
    this->firstAncestorOrThisOfType(parent);
    if (parent)
    {
        parent->updateCurveNamesAndPlotTitle();
        parent->reattachCurvesToQwtAndReplot();
        parent->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::updateCurveNames(size_t curveSetIndex, size_t curveSetCount)
{
    for (size_t i = 0; i < m_crossPlotCurves.size(); ++i)
    {
        QString curveSetName = createAutoName();
        if (curveSetName.isEmpty())
        {
            if (curveSetCount > 1u)
                curveSetName = QString("Curve Set #%1").arg(curveSetIndex + 1);
            else
                curveSetName = "Curve Set";
        }

        auto curve = m_crossPlotCurves[i];
        if (groupingEnabled())
        {
            QString curveGroupName = createGroupName(curve->groupIndex());
            curve->setCustomName(curveGroupName);
            curve->setLegendEntryText(curveSetName);
        }
        else
        {
            curve->setCustomName(curveSetName);
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
            m_case                      = eclipseCase;
            m_xAxisProperty->setEclipseCase(eclipseCase);
            m_yAxisProperty->setEclipseCase(eclipseCase);
            m_groupingProperty->setEclipseCase(eclipseCase);

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
    if (groupingEnabled())
    {
        m_groupingProperty->uiTreeOrdering(uiTreeOrdering, uiConfigName);
    }

    for (auto curve : m_crossPlotCurves())
    {
        uiTreeOrdering.add(curve);
    }

    uiTreeOrdering.add(&m_plotCellFilterCollection);

    uiTreeOrdering.skipRemainingChildren(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGridCrossPlotCurveSet::hasMultipleTimeSteps() const
{
    return m_timeStep() == -1 && (m_xAxisProperty->hasDynamicResult() || m_yAxisProperty->hasDynamicResult());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::filterInvalidCurveValues(RigEclipseCrossPlotResult* result)
{
    bool xLog = isXAxisLogarithmic();
    bool yLog = isYAxisLogarithmic();

    if (xLog || yLog)
    {
        RigEclipseCrossPlotResult validResult;
        for (size_t i = 0; i < result->xValues.size(); ++i)
        {
            double xValue  = result->xValues[i];
            double yValue  = result->yValues[i];
            bool   invalid = (xLog && xValue <= 0.0) || (yLog && yValue <= 0.0);
            if (!invalid)
            {
                validResult.xValues.push_back(xValue);
                validResult.yValues.push_back(yValue);
                if (i < result->groupValuesContinuous.size())
                {
                    validResult.groupValuesContinuous.push_back(result->groupValuesContinuous[i]);
                }
                if (i < result->groupValuesDiscrete.size())
                {
                    validResult.groupValuesDiscrete.push_back(result->groupValuesDiscrete[i]);
                }
            }
        }

        *result = validResult;
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

    CAF_PDM_InitField(&addCaseName, "AddCaseName", true, "Add Case Name", "", "", "");
    CAF_PDM_InitField(&addAxisVariables, "AddAxisVariables", true, "Add Axis Variables", "", "", "");
    CAF_PDM_InitField(&addTimestep, "AddTimeStep", true, "Add Time Step", "", "", "");
    CAF_PDM_InitField(&addGrouping, "AddGrouping", true, "Add Data Group", "", "", "");

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
    uiOrdering.add(&addGrouping);
}
