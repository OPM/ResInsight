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
#include "RigFormationNames.h"
#include "RigMainGrid.h"

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotCurve.h"
#include "RimProject.h"
#include "RimTools.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiSliderEditor.h"

CAF_PDM_SOURCE_INIT(RimGridCrossPlotCurveSet, "GridCrossPlotCurveSet");



namespace caf
{
template<>
void AppEnum<RimGridCrossPlotCurveSet::CurveCategorization>::setUp()
{
    addItem(RimGridCrossPlotCurveSet::NO_CATEGORIZATION, "NONE", "None");
    addItem(RimGridCrossPlotCurveSet::TIME_CATEGORIZATION, "TIME", "Time");
    addItem(RimGridCrossPlotCurveSet::FORMATION_CATEGORIZATION, "FORMATION", "Formations");
    addItem(RimGridCrossPlotCurveSet::RESULT_CATEGORIZATION, "RESULT", "Result Property");
    setDefault(RimGridCrossPlotCurveSet::TIME_CATEGORIZATION);
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

class RigEclipseResultBinSorter
{
public:
    RigEclipseResultBinSorter(const std::vector<std::vector<double>>& allDataValues, int binCount)
        : m_allDataValues(allDataValues)
        , m_binCount(binCount)
        , m_minValue(std::numeric_limits<double>::infinity())
        , m_maxValue(-std::numeric_limits<double>::infinity())
        , m_binSize(0.0)
    {
        calculateRange();
    }

    int binNumber(double value) const
    {
        double distFromMin = value - m_minValue;        
        return std::min(m_binCount - 1, static_cast<int>(distFromMin / m_binSize));
    }

    std::pair<double, double> binRange(int binNumber)
    {
        double minBinValue = m_minValue + m_binSize * binNumber;
        double maxBinBalue = minBinValue + m_binSize;
        return std::make_pair(minBinValue, maxBinBalue);
    }

private:
    void calculateRange()
    {
        for (const std::vector<double>& doubleRange : m_allDataValues)
        {
            if (!doubleRange.empty())
            {
                for (double value : doubleRange)
                {
                    if (value != std::numeric_limits<double>::infinity())
                    {
                        m_minValue = std::min(m_minValue, value);
                        m_maxValue = std::max(m_maxValue, value);
                    }
                }
            }
        }

        if (m_maxValue > m_minValue)
        {
            m_binSize = (m_maxValue - m_minValue) / m_binCount;
        }
    }
private:
    const std::vector<std::vector<double>>& m_allDataValues;
    int                                     m_binCount;
    double                                  m_minValue;
    double                                  m_maxValue;
    double                                  m_binSize;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGridCrossPlotCurveSet::onLoadDataAndUpdate(bool updateParentPlot)
{
    performAutoNameUpdate();
    
    detachAllCurves();
    m_crossPlotCurves.deleteAllChildObjects();

    std::map<int, QVector<QPointF>> samples;

    if (m_case())
    {
        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case.value());

        if (eclipseCase)
        {
            if (!eclipseCase->ensureReservoirCaseIsOpen())
            {
                RiaLogging::warning(QString("Failed to open eclipse grid file %1").arg(eclipseCase->gridFileName()));

                return;
            }

            RigCaseCellResultsData* resultData = eclipseCase->results(RiaDefines::MATRIX_MODEL);
            RigFormationNames* activeFormationNames = resultData->activeFormationNames();

            RigEclipseResultAddress xAddress(m_xAxisProperty->resultType(), m_xAxisProperty->resultVariable());
            RigEclipseResultAddress yAddress(m_yAxisProperty->resultType(), m_yAxisProperty->resultVariable());
            RigEclipseResultAddress catAddress(m_categoryProperty->resultType(), m_categoryProperty->resultVariable());

            std::unique_ptr<RigEclipseResultBinSorter> catBinSorter;
            const std::vector<std::vector<double>>*    catValuesForAllSteps = nullptr;

            if (xAddress.isValid() && yAddress.isValid())
            {
                RigActiveCellInfo* activeCellInfo = resultData->activeCellInfo();
                const RigMainGrid*       mainGrid = eclipseCase->mainGrid();

                resultData->ensureKnownResultLoaded(xAddress);
                resultData->ensureKnownResultLoaded(yAddress);

                const std::vector<std::vector<double>>& xValuesForAllSteps = resultData->cellScalarResults(xAddress);
                const std::vector<std::vector<double>>& yValuesForAllSteps = resultData->cellScalarResults(yAddress);

                if (m_categorization() == RESULT_CATEGORIZATION && catAddress.isValid())
                {
                    resultData->ensureKnownResultLoaded(catAddress);
                    catValuesForAllSteps = &resultData->cellScalarResults(catAddress);
                    catBinSorter.reset(new RigEclipseResultBinSorter(*catValuesForAllSteps, m_categoryBinCount));
                }

                std::set<int> timeStepsToInclude;
                if (m_timeStep() == -1)
                {
                    size_t nStepsInData = std::max(xValuesForAllSteps.size(), yValuesForAllSteps.size());
                    CVF_ASSERT(xValuesForAllSteps.size() == 1u || xValuesForAllSteps.size() == nStepsInData);
                    CVF_ASSERT(yValuesForAllSteps.size() == 1u || yValuesForAllSteps.size() == nStepsInData);
                    for (size_t i = 0; i < nStepsInData; ++i)
                    {
                        timeStepsToInclude.insert((int)i);
                    }
                }
                else
                {
                    timeStepsToInclude.insert(static_cast<size_t>(m_timeStep()));
                }

                for (int timeStep : timeStepsToInclude)
                {
                    int xIndex = timeStep >= (int)xValuesForAllSteps.size() ? 0 : timeStep;
                    int yIndex = timeStep >= (int)yValuesForAllSteps.size() ? 0 : timeStep;                    

                    RigActiveCellsResultAccessor xAccessor(mainGrid, &xValuesForAllSteps[xIndex], activeCellInfo);
                    RigActiveCellsResultAccessor yAccessor(mainGrid, &yValuesForAllSteps[yIndex], activeCellInfo);
                    std::unique_ptr<RigActiveCellsResultAccessor> catAccessor;
                    if (catValuesForAllSteps)
                    {
                        int catIndex = timeStep >= (int)catValuesForAllSteps->size() ? 0 : timeStep;
                        catAccessor.reset(new RigActiveCellsResultAccessor(mainGrid, &(catValuesForAllSteps->at(catIndex)), activeCellInfo));
                    }

                    for (size_t globalCellIdx = 0; globalCellIdx < activeCellInfo->reservoirCellCount(); ++globalCellIdx)
                    {
                        double xValue = xAccessor.cellScalarGlobIdx(globalCellIdx);
                        double yValue = yAccessor.cellScalarGlobIdx(globalCellIdx);

                        int category = 0;
                        if (m_categorization() == TIME_CATEGORIZATION)
                        {
                            category = timeStep;
                        }
                        else if (m_categorization() == FORMATION_CATEGORIZATION && activeFormationNames)
                        {
                            size_t i(cvf::UNDEFINED_SIZE_T), j(cvf::UNDEFINED_SIZE_T), k(cvf::UNDEFINED_SIZE_T);
                            if (mainGrid->ijkFromCellIndex(globalCellIdx, &i, &j, &k))
                            {
                                category = activeFormationNames->formationIndexFromKLayerIdx(k);
                            }
                        }
                        else if (catAccessor && catBinSorter)
                        {
                            double catValue = catAccessor->cellScalarGlobIdx(globalCellIdx);
                            category = catBinSorter->binNumber(catValue);
                        }
                        if (xValue != HUGE_VAL && yValue != HUGE_VAL)
                        {
                            samples[category].push_back(QPointF(xValue, yValue));
                        }
                    }
                }
            }

            QStringList timeStepNames = m_case->timeStepStrings();

            int curveSetIndex = indexInPlot();

            for (const auto& sampleCategory : samples)
            {
                RimGridCrossPlotCurve* curve = new RimGridCrossPlotCurve();
                QString categoryName;
                if (m_categorization() == TIME_CATEGORIZATION)
                {
                    bool staticResultsOnly = staticResultsOnly = m_xAxisProperty->hasStaticResult() && m_yAxisProperty->hasStaticResult();
                    if (!staticResultsOnly && sampleCategory.first < timeStepNames.size())
                    {
                        categoryName = timeStepNames[sampleCategory.first];
                    }
                }
                else if (m_categorization() == FORMATION_CATEGORIZATION && activeFormationNames)
                {
                    categoryName = activeFormationNames->formationNameFromKLayerIdx(sampleCategory.first);
                }
                else if (catBinSorter)
                {
                    std::pair<double, double> binRange = catBinSorter->binRange(sampleCategory.first);
                    categoryName = QString("%1 [%2, %3]").arg(m_categoryProperty->resultVariableUiShortName()).arg(binRange.first).arg(binRange.second);
                }

                if (categoryName.isEmpty())
                {
                    curve->setCustomName(createAutoName());
                }
                else
                {
                    curve->setCustomName(QString("%1 : %2").arg(createAutoName()).arg(categoryName));
                }
                curve->determineColorAndSymbol(curveSetIndex, sampleCategory.first, (int)samples.size(), m_categorization() == FORMATION_CATEGORIZATION);
                curve->setSamples(sampleCategory.second);
                curve->updateCurveAppearance();
                curve->updateCurveNameAndUpdatePlotLegendAndTitle();
                curve->updateUiIconFromPlotSymbol();

                m_crossPlotCurves.push_back(curve);
            }
        }
    }

    if (updateParentPlot)
    {
        triggerReplotAndTreeRebuild();
    }
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
        RimTools::caseOptionItems(&options);
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
