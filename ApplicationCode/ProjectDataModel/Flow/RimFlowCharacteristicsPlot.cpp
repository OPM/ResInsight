/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimFlowCharacteristicsPlot.h"

#include "RigFlowDiagResults.h"

#include "RimEclipseResultCase.h"
#include "RimFlowDiagSolution.h"
#include "RimProject.h"

#include "RiuFlowCharacteristicsPlot.h"

#include <cmath> // Needed for HUGE_VAL on Linux


namespace caf
{
template<>
void AppEnum< RimFlowCharacteristicsPlot::TimeSelectionType >::setUp()
{
    addItem(RimFlowCharacteristicsPlot::ALL_AVAILABLE,    "ALL_AVAILABLE",    "All available");
    addItem(RimFlowCharacteristicsPlot::SELECT_AVAILABLE, "SELECT_AVAILABLE",        "Select");
    setDefault(RimFlowCharacteristicsPlot::ALL_AVAILABLE);
}
}

CAF_PDM_SOURCE_INIT(RimFlowCharacteristicsPlot, "FlowCharacteristicsPlot");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFlowCharacteristicsPlot::RimFlowCharacteristicsPlot()
{
    CAF_PDM_InitObject("Flow Characteristics", ":/WellAllocPie16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_case, "FlowCase", "Case", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_flowDiagSolution, "FlowDiagSolution", "Flow Diag Solution", "", "", "");
    m_flowDiagSolution.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_timeStepSelectionType, "TimeSelectionType", "Time Steps", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_selectedTimeSteps, "SelectedTimeSteps", "", "", "", "");

    this->m_showWindow = false;
    setAsPlotMdiWindow();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFlowCharacteristicsPlot::~RimFlowCharacteristicsPlot()
{
    removeMdiWindowFromMdiArea();
    
    deleteViewWidget();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFlowCharacteristicsPlot::setFromFlowSolution(RimFlowDiagSolution* flowSolution)
{
    if ( !flowSolution )
    {
        m_case = nullptr;
    }
    else
    {
        RimEclipseResultCase* eclCase;
        flowSolution->firstAncestorOrThisOfType(eclCase);
        m_case = eclCase;
    }

    m_flowDiagSolution = flowSolution;
    m_showWindow = true;

    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFlowCharacteristicsPlot::deleteViewWidget()
{
    if (m_flowCharPlotWidget)
    {
        m_flowCharPlotWidget->deleteLater();
        m_flowCharPlotWidget= nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFlowCharacteristicsPlot::updateCurrentTimeStep()
{
    if (m_timeStepSelectionType() != ALL_AVAILABLE) return;
    if (!m_flowDiagSolution()) return;

    RigFlowDiagResults* flowResult = m_flowDiagSolution->flowDiagResults();
    std::vector<int> calculatedTimesteps = flowResult->calculatedTimeSteps();
    
    if (m_currentlyPlottedTimeSteps == calculatedTimesteps) return;

    this->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimFlowCharacteristicsPlot::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_case )
    {
        RimProject* proj = nullptr;
        this->firstAncestorOrThisOfType(proj);
        if ( proj )
        {
            std::vector<RimEclipseResultCase*> cases;
            proj->descendantsIncludingThisOfType(cases);

            for ( RimEclipseResultCase* c : cases )
            {
                if ( c->defaultFlowDiagSolution() )
                {
                    options.push_back(caf::PdmOptionItemInfo(c->caseUserDescription(), c, false, c->uiIcon()));
                }
            }
        }
    }
    else if ( fieldNeedingOptions == &m_flowDiagSolution )
    {
        if ( m_case )
        {
            std::vector<RimFlowDiagSolution*> flowSols = m_case->flowDiagSolutions();

            options.push_back(caf::PdmOptionItemInfo("None", nullptr));
            for ( RimFlowDiagSolution* flowSol : flowSols )
            {
                options.push_back(caf::PdmOptionItemInfo(flowSol->userDescription(), flowSol, false, flowSol->uiIcon()));
            }
        }
    }
    else if ( fieldNeedingOptions == &m_selectedTimeSteps )
    {
        if ( m_flowDiagSolution )
        {
            RigFlowDiagResults* flowResult = m_flowDiagSolution->flowDiagResults();
            std::vector<int> calculatedTimesteps = flowResult->calculatedTimeSteps();

            std::vector<QDateTime> timeStepDates = m_case->timeStepDates();

            for ( int tsIdx : calculatedTimesteps )
            {
                options.push_back(caf::PdmOptionItemInfo(timeStepDates[tsIdx].toString(), tsIdx));
            }
        }
    }

    return options;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFlowCharacteristicsPlot::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_case);
    uiOrdering.add(&m_flowDiagSolution);
    uiOrdering.add(&m_timeStepSelectionType);

    if (m_timeStepSelectionType == SELECT_AVAILABLE) uiOrdering.add(&m_selectedTimeSteps);

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RimFlowCharacteristicsPlot::viewWidget()
{
    return m_flowCharPlotWidget;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFlowCharacteristicsPlot::zoomAll()
{
    if (m_flowCharPlotWidget) m_flowCharPlotWidget->zoomAll();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFlowCharacteristicsPlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimViewWindow::fieldChangedByUi(changedField, oldValue, newValue);

    if ( &m_case == changedField )
    {
        m_flowDiagSolution = m_case->defaultFlowDiagSolution();  
        m_currentlyPlottedTimeSteps.clear();  
    }

    // All fields update plot

    this->loadDataAndUpdate();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QImage RimFlowCharacteristicsPlot::snapshotWindowContent()
{
    QImage image;

    if (m_flowCharPlotWidget)
    {
        QPixmap pix = QPixmap::grabWidget(m_flowCharPlotWidget);
        image = pix.toImage();
    }

    return image;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFlowCharacteristicsPlot::loadDataAndUpdate()
{
    updateMdiWindowVisibility();

    if (m_flowDiagSolution && m_flowCharPlotWidget)
    {
        RigFlowDiagResults* flowResult = m_flowDiagSolution->flowDiagResults();
        std::vector<int> calculatedTimesteps = flowResult->calculatedTimeSteps();
        
        if (m_timeStepSelectionType == SELECT_AVAILABLE)
        {
            // Find set intersection of selected and available time steps
            std::set<int> calculatedTimeStepsSet;
            calculatedTimeStepsSet.insert(calculatedTimesteps.begin(), calculatedTimesteps.end());
            calculatedTimesteps.clear();

            auto selectedTimeSteps = m_selectedTimeSteps();
            for (int tsIdx : selectedTimeSteps)
            { 
                 if (calculatedTimeStepsSet.count(tsIdx)) calculatedTimesteps.push_back(tsIdx);
            }
        }
        
        m_currentlyPlottedTimeSteps = calculatedTimesteps;

        std::vector<QDateTime> timeStepDates = m_case->timeStepDates();
        std::vector<double> lorenzVals(timeStepDates.size(), HUGE_VAL);

        m_flowCharPlotWidget->removeAllCurves();

        for ( int timeStepIdx: calculatedTimesteps )
        {
            lorenzVals[timeStepIdx] = flowResult->flowCharacteristicsResults(timeStepIdx).m_lorenzCoefficient;
        }
        m_flowCharPlotWidget->setLorenzCurve(timeStepDates, lorenzVals);

        for ( int timeStepIdx: calculatedTimesteps )
        {

            const auto & flowCharResults = flowResult->flowCharacteristicsResults(timeStepIdx);
            m_flowCharPlotWidget->addFlowCapStorageCapCurve(timeStepDates[timeStepIdx],
                                                            flowCharResults.m_flowCapStorageCapCurve.first,
                                                            flowCharResults.m_flowCapStorageCapCurve.second);
            m_flowCharPlotWidget->addSweepEfficiencyCurve(timeStepDates[timeStepIdx],
                                                          flowCharResults.m_sweepEfficiencyCurve.first,
                                                          flowCharResults.m_sweepEfficiencyCurve.second);
        }

    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RimFlowCharacteristicsPlot::createViewWidget(QWidget* mainWindowParent)
{
    m_flowCharPlotWidget = new RiuFlowCharacteristicsPlot(this, mainWindowParent);
    return m_flowCharPlotWidget;
}


