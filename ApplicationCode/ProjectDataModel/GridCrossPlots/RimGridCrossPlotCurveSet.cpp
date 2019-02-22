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
#include "RigMainGrid.h"

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotCurve.h"
#include "RimProject.h"
#include "RimTools.h"

#include "cafPdmUiComboBoxEditor.h"

CAF_PDM_SOURCE_INIT(RimGridCrossPlotCurveSet, "GridCrossPlotCurveSet");

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

    CAF_PDM_InitFieldNoDefault(&m_xAxisProperty, "XAxisProperty", "X-Axis Property", "", "", "");
    m_xAxisProperty = new RimEclipseResultDefinition;
    m_xAxisProperty.uiCapability()->setUiHidden(true);
    m_xAxisProperty.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_yAxisProperty, "YAxisProperty", "Y-Axis Property", "", "", "");
    m_yAxisProperty = new RimEclipseResultDefinition;
    m_yAxisProperty.uiCapability()->setUiHidden(true);
    m_yAxisProperty.uiCapability()->setUiTreeChildrenHidden(true);

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
void RimGridCrossPlotCurveSet::initAfterRead()
{
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(m_case());
    if (eclipseCase)
    {
        m_xAxisProperty->setEclipseCase(eclipseCase);
        m_yAxisProperty->setEclipseCase(eclipseCase);
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

            RigEclipseResultAddress xAddress(m_xAxisProperty->resultType(), m_xAxisProperty->resultVariable());
            RigEclipseResultAddress yAddress(m_yAxisProperty->resultType(), m_yAxisProperty->resultVariable());            

            if (xAddress.isValid() && yAddress.isValid())
            {
                RigActiveCellInfo* activeCellInfo = resultData->activeCellInfo();
                const RigMainGrid*       mainGrid = eclipseCase->mainGrid();

                resultData->ensureKnownResultLoaded(xAddress);
                resultData->ensureKnownResultLoaded(yAddress);

                const std::vector<std::vector<double>>& xValuesForAllSteps = resultData->cellScalarResults(xAddress);
                const std::vector<std::vector<double>>& yValuesForAllSteps = resultData->cellScalarResults(yAddress);

                std::set<int> timeStepsToInclude;
                if (m_timeStep() == -1)
                {
                    size_t nStepsInData = std::max(xValuesForAllSteps.size(), yValuesForAllSteps.size());
                    CVF_ASSERT(xValuesForAllSteps.size() == 1u || xValuesForAllSteps.size() == nStepsInData);
                    CVF_ASSERT(yValuesForAllSteps.size() == 1u || yValuesForAllSteps.size() == nStepsInData);
                    for (size_t i = 0; i < nStepsInData; ++i)
                    {
                        timeStepsToInclude.insert((int) i);
                    }
                }
                else
                {
                    timeStepsToInclude.insert(static_cast<size_t>(m_timeStep()));
                }

                for (int timeStep : timeStepsToInclude)
                {
                    int xIndex = timeStep >= (int) xValuesForAllSteps.size() ? 0 : timeStep;
                    int yIndex = timeStep >= (int) yValuesForAllSteps.size() ? 0 : timeStep;

                    RigActiveCellsResultAccessor xAccessor(mainGrid, &xValuesForAllSteps[xIndex], activeCellInfo);
                    RigActiveCellsResultAccessor yAccessor(mainGrid, &yValuesForAllSteps[yIndex], activeCellInfo);
                    for (size_t j = 0; j < activeCellInfo->reservoirCellCount(); ++j)
                    {
                        double xValue = xAccessor.cellScalarGlobIdx(j);
                        double yValue = yAccessor.cellScalarGlobIdx(j);
                        if (xValue != HUGE_VAL && yValue != HUGE_VAL)
                        {
                            samples[timeStep].push_back(QPointF(xValue, yValue));
                        }
                    }
                }
            }
        }
    }

    QStringList timeStepNames;

    if (m_case)
    {
        timeStepNames = m_case->timeStepStrings();
    }

    int curveSetIndex = indexInPlot();

    for (const auto& sampleCategory : samples)
    {
        RimGridCrossPlotCurve* curve = new RimGridCrossPlotCurve();
        QString timeStepName = QString::number(sampleCategory.first);
        if (sampleCategory.first < timeStepNames.size())
        {
            timeStepName = timeStepNames[sampleCategory.first];
        }
        bool staticResultsOnly = staticResultsOnly = m_xAxisProperty->hasStaticResult() && m_yAxisProperty->hasStaticResult();
        if (staticResultsOnly)
        {
            curve->setCustomName(createAutoName());
        }
        else
        {
            curve->setCustomName(QString("%1 : %2").arg(createAutoName()).arg(timeStepName));
        }
        curve->determineColorAndSymbol(curveSetIndex, sampleCategory.first, (int) samples.size(), false);
        curve->setSamples(sampleCategory.second);
        curve->updateCurveAppearance();
        curve->updateCurveNameAndUpdatePlotLegendAndTitle();
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
void RimGridCrossPlotCurveSet::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_case);
    if (m_case)
    {
        uiOrdering.add(&m_timeStep);

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
            m_xAxisProperty->updateConnectedEditors();
            m_yAxisProperty->updateConnectedEditors();
            triggerReplotAndTreeRebuild();
        }
    }
    else if (changedField == &m_timeStep)
    {
        triggerReplotAndTreeRebuild();
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
    parent->loadDataAndUpdate();
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

            m_xAxisProperty->setResultType(RiaDefines::DYNAMIC_NATIVE);
            m_xAxisProperty->setResultVariable("SOIL");

            m_yAxisProperty->setResultType(RiaDefines::DYNAMIC_NATIVE);
            m_yAxisProperty->setResultVariable("PRESSURE");
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
}
