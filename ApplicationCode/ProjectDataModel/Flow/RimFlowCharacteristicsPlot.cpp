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

#include "RimFlowDiagSolution.h"
#include "RimEclipseResultCase.h"

#include "RigFlowDiagResults.h"

#include "RiuFlowCharacteristicsPlot.h"
#include "RimProject.h"


CAF_PDM_SOURCE_INIT(RimFlowCharacteristicsPlot, "FlowCharacteristicsPlot");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFlowCharacteristicsPlot::RimFlowCharacteristicsPlot()
{
    CAF_PDM_InitObject("Flow Characteristics", ":/WellAllocPie16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_case, "FlowCase", "Case", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_flowDiagSolution, "FlowDiagSolution", "Flow Diag Solution", "", "", "");

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
                options.push_back(caf::PdmOptionItemInfo(c->caseUserDescription(), c, false, c->uiIcon()));
            }
        }
    }
    else if ( fieldNeedingOptions == &m_flowDiagSolution )
    {
        if ( m_case )
        {
            std::vector<RimFlowDiagSolution*> flowSols = m_case->flowDiagSolutions();

            for ( RimFlowDiagSolution* flowSol : flowSols )
            {
                options.push_back(caf::PdmOptionItemInfo("None", nullptr));
                options.push_back(caf::PdmOptionItemInfo(flowSol->userDescription(), flowSol, false, flowSol->uiIcon()));
            }
        }
    }

    return options;

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
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFlowCharacteristicsPlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimViewWindow::fieldChangedByUi(changedField, oldValue, newValue);

    if (   &m_case == changedField 
        || &m_flowDiagSolution == changedField)
    {
        this->loadDataAndUpdate();
    }

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

    if (m_flowDiagSolution)
    {
        RigFlowDiagResults* flowResult = m_flowDiagSolution->flowDiagResults();
        std::vector<int> calculatedTimesteps = flowResult->calculatedTimeSteps();
        
        std::vector<QDateTime> timeStepDates = m_case->timeStepDates();
        std::vector<double> lorenzVals(timeStepDates.size(), HUGE_VAL);

        for (int timeStepIdx: calculatedTimesteps)
        {
            lorenzVals[timeStepIdx] = flowResult->flowCharacteristicsResults(timeStepIdx).m_lorenzCoefficient; 

            if ( m_flowCharPlotWidget) 
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

       if ( m_flowCharPlotWidget) m_flowCharPlotWidget->setLorenzCurve(timeStepDates, lorenzVals);
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


