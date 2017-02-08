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

#include "RimWellAllocationPlot.h"

#include "RiaApplication.h"

#include "RigEclipseCaseData.h"
#include "RigSimulationWellCenterLineCalculator.h"
#include "RigSimulationWellCoordsAndMD.h"
#include "RigSingleWellResultsData.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimEclipseWellCollection.h"
#include "RimFlowDiagSolution.h"
#include "RimTotalWellAllocationPlot.h"
#include "RimWellFlowRateCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"

#include "RiuMainPlotWindow.h"
#include "RiuWellAllocationPlot.h"
#include "RigAccWellFlowCalculator.h"
#include "RimProject.h"

CAF_PDM_SOURCE_INIT(RimWellAllocationPlot, "WellAllocationPlot");



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellAllocationPlot::RimWellAllocationPlot()
{
    CAF_PDM_InitObject("Well Allocation Plot", ":/newIcon16x16.png", "", "");

    CAF_PDM_InitField(&m_userName, "PlotDescription", QString("Flow Diagnostics Plot"), "Name", "", "", "");
    m_userName.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&m_showPlotTitle, "ShowPlotTitle", true, "Show Plot Title", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_case, "CurveCase", "Case", "", "", "");
    m_case.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitField(&m_timeStep, "PlotTimeStep", 0, "Time Step", "", "", "");
    CAF_PDM_InitField(&m_wellName, "WellName", QString("None"), "Well", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_flowDiagSolution, "FlowDiagSolution", "Flow Diag Solution", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_accumulatedWellFlowPlot, "AccumulatedWellFlowPlot", "Accumulated Well Flow", "", "", "");
    m_accumulatedWellFlowPlot.uiCapability()->setUiHidden(true);
    m_accumulatedWellFlowPlot = new RimWellLogPlot;

    CAF_PDM_InitFieldNoDefault(&m_totalWellAllocationPlot, "TotalWellFlowPlot", "Total Well Flow", "", "", "");
    m_totalWellAllocationPlot.uiCapability()->setUiHidden(true);

    m_totalWellAllocationPlot = new RimTotalWellAllocationPlot;

    this->setAsPlotMdiWindow();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellAllocationPlot::~RimWellAllocationPlot()
{
    removeMdiWindowFromMdiArea();
    
    delete m_accumulatedWellFlowPlot();
    delete m_totalWellAllocationPlot();

    deleteViewWidget();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::setFromSimulationWell(RimEclipseWell* simWell)
{
    RimEclipseView* eclView;
    simWell->firstAncestorOrThisOfType(eclView);
    RimEclipseResultCase* eclCase;
    simWell->firstAncestorOrThisOfType(eclCase);

    m_case = eclCase;
    m_wellName = simWell->wellResults()->m_wellName;
    m_timeStep = eclView->currentTimeStep();

    m_flowDiagSolution = eclView->cellResult()->flowDiagSolution();

    updateFromWell();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::deleteViewWidget()
{
    if (m_wellAllocationPlotWidget)
    {
        m_wellAllocationPlotWidget->deleteLater();
        m_wellAllocationPlotWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::updateFromWell()
{
    // Delete existing tracks
    {
        std::vector<RimWellLogTrack*> tracks;
        accumulatedWellFlowPlot()->descendantsIncludingThisOfType(tracks);

        for (RimWellLogTrack* t : tracks)
        {
            accumulatedWellFlowPlot()->removeTrack(t);
            delete t;
        }
    }

    CVF_ASSERT(accumulatedWellFlowPlot()->trackCount() == 0);

    accumulatedWellFlowPlot()->setDescription("Accumulated Well Flow (" + m_wellName + ")");

    if (!m_case) return;

    const RigSingleWellResultsData* wellResults = nullptr;
    wellResults = m_case->reservoirData()->findWellResult(m_wellName);

    if (!wellResults) return;
   
   // Set up the Accumulated Well Flow Calculator

    std::vector< std::vector <cvf::Vec3d> > pipeBranchesCLCoords;
    std::vector< std::vector <RigWellResultPoint> > pipeBranchesCellIds;

    RigSimulationWellCenterLineCalculator::calculateWellPipeCenterlineFromWellFrame(m_case->reservoirData(),
                                                                                    wellResults,
                                                                                    m_timeStep,
                                                                                    true,
                                                                                    true,
                                                                                    pipeBranchesCLCoords,
                                                                                    pipeBranchesCellIds);
    std::unique_ptr< RigAccWellFlowCalculator > wfCalculator;
    std::map<QString, const std::vector<double>* > tracerCellFractionValues;

    if ( m_flowDiagSolution && wellResults->hasWellResult(m_timeStep))
    {
        RimFlowDiagSolution::TracerStatusType requestedTracerType = RimFlowDiagSolution::UNDEFINED;

        const RigWellResultFrame& wellResultFrame = wellResults->wellResultFrame(m_timeStep);
        if (wellResultFrame.m_productionType == RigWellResultFrame::PRODUCER)
        {
            requestedTracerType = RimFlowDiagSolution::INJECTOR;
        }
        else
        {
            requestedTracerType = RimFlowDiagSolution::PRODUCER;
        }


        std::vector<QString> tracerNames = m_flowDiagSolution->tracerNames();
        for ( const QString& tracerName : tracerNames )
        {
            if (m_flowDiagSolution->tracerStatusInTimeStep(tracerName, m_timeStep) == requestedTracerType)
            {
                RigFlowDiagResultAddress resAddr(RIG_FLD_CELL_FRACTION_RESNAME, tracerName.toStdString());
                const std::vector<double>* tracerCellFractions = m_flowDiagSolution->flowDiagResults()->resultValues(resAddr, m_timeStep);
                tracerCellFractionValues[tracerName] = tracerCellFractions;
            }
        }

        if ( tracerCellFractionValues.size() )
        {
            RigEclCellIndexCalculator cellIdxCalc(m_case->reservoirData()->mainGrid(), m_case->reservoirData()->activeCellInfo(RifReaderInterface::MATRIX_RESULTS));
            wfCalculator.reset(new RigAccWellFlowCalculator(pipeBranchesCLCoords,
                                                            pipeBranchesCellIds,
                                                            tracerCellFractionValues,
                                                            cellIdxCalc));
        }
    }

    if (!wfCalculator)
    {
        if (pipeBranchesCLCoords.size() > 0)
        {
            wfCalculator.reset(new RigAccWellFlowCalculator(pipeBranchesCLCoords,
                                                  pipeBranchesCellIds));
        }
    }

    // Create tracks and curves from the calculated data

    size_t branchCount = pipeBranchesCLCoords.size();
    for (size_t brIdx = 0; brIdx < branchCount; ++brIdx)
    {
        // Skip Tiny dummy branches
        if (pipeBranchesCellIds[brIdx].size() <= 3) continue;

        RimWellLogTrack* plotTrack = new RimWellLogTrack();

        // TODO: Description is overwritten by RimWellLogPlot::updateTrackNames()
        // Add flag to control if this behavior
        plotTrack->setDescription(QString("Branch %1").arg(brIdx + 1));

        accumulatedWellFlowPlot()->addTrack(plotTrack);

        std::vector<double> connNumbers;
        {
            const std::vector<size_t>& connNumbersSize_t = wfCalculator->connectionNumbersFromTop(brIdx);
            for ( size_t conNumb : connNumbersSize_t ) connNumbers.push_back(static_cast<double>(conNumb));
        }

        if ( m_flowDiagSolution )
        {
            std::vector<QString> tracerNames = wfCalculator->tracerNames();
            for (const QString& tracerName: tracerNames)
            {
                const std::vector<double>& accFlow = wfCalculator->accumulatedTracerFlowPrConnection(tracerName, brIdx); 
                addStackedCurve(tracerName, connNumbers, accFlow, plotTrack);
            }
        }
        else
        {
            const std::vector<double>& accFlow = wfCalculator->accumulatedTotalFlowPrConnection(brIdx);
            addStackedCurve("Total", connNumbers, accFlow, plotTrack);
        }

    }

    QString wellStatusText = QString("(%1)").arg(RimWellAllocationPlot::wellStatusTextForTimeStep(m_wellName, m_case, m_timeStep));
    
    setDescription("Well Allocation: " + m_wellName + " " + wellStatusText + ", " +  m_case->timeStepStrings()[m_timeStep] + " (" + m_case->caseUserDescription() + ")");
 
    /// Pie chart

    m_totalWellAllocationPlot->clearSlices();

    if (wfCalculator)
    {
        std::vector<QString> tracerNames =  wfCalculator->tracerNames();
        std::vector<std::pair<QString, double> > tracerWithValues;

        for (const QString& tracerName: tracerNames)
        {
            const std::vector<double>& accFlow = wfCalculator->accumulatedTracerFlowPrConnection(tracerName, 0);
            tracerWithValues.push_back(std::make_pair(tracerName, accFlow.back()));
        }

        float sumTracerVals = 0.0f;
        for ( const auto& tracerVal:tracerWithValues)
        {
             sumTracerVals += tracerVal.second;
        }

        if ( sumTracerVals != 0.0f )
        {
            for ( const auto& tracerVal : tracerWithValues )
            {
                cvf::Color3f color;
                if ( m_flowDiagSolution )
                    color = m_flowDiagSolution->tracerColor(tracerVal.first);
                else
                    color = cvf::Color3f::DARK_GRAY;

                m_totalWellAllocationPlot->addSlice(tracerVal.first, color, 100*tracerVal.second/sumTracerVals);
            }
        }
    }
    
    m_totalWellAllocationPlot->updateConnectedEditors();

    accumulatedWellFlowPlot()->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::addStackedCurve(const QString& tracerName, 
                                            const std::vector<double>& connNumbers, 
                                            const std::vector<double>& accFlow, 
                                            RimWellLogTrack* plotTrack)
{
    RimWellFlowRateCurve* curve = new RimWellFlowRateCurve;
    curve->setFlowValues(tracerName, connNumbers, accFlow);

    if ( m_flowDiagSolution )
        curve->setColor(m_flowDiagSolution->tracerColor(tracerName));
    else
        curve->setColor(cvf::Color3f::DARK_GRAY);
    
    plotTrack->addCurve(curve);

    curve->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::updateWidgetTitleWindowTitle()
{
    updateMdiWindowTitle();

    if (m_wellAllocationPlotWidget)
    {
        if (m_showPlotTitle)
        {
            m_wellAllocationPlotWidget->showTitle(m_userName);
        }
        else
        {
            m_wellAllocationPlotWidget->hideTitle();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellAllocationPlot::wellStatusTextForTimeStep(const QString& wellName, const RimEclipseResultCase* eclipseResultCase, size_t timeStep)
{
    QString statusText = "Undefined";

    if (eclipseResultCase)
    {
        const RigSingleWellResultsData* wellResults = nullptr;
        wellResults = eclipseResultCase->reservoirData()->findWellResult(wellName);

        if (wellResults)
        {
            if (wellResults->hasWellResult(timeStep))
            {
                const RigWellResultFrame& wellResultFrame = wellResults->wellResultFrame(timeStep);

                RigWellResultFrame::WellProductionType prodType = wellResultFrame.m_productionType;

                switch (prodType)
                {
                case RigWellResultFrame::PRODUCER:
                    statusText = "Producer";
                    break;
                case RigWellResultFrame::OIL_INJECTOR:
                    statusText = "Oil Injector";
                    break;
                case RigWellResultFrame::GAS_INJECTOR:
                    statusText = "Gas Injector";
                    break;
                case RigWellResultFrame::WATER_INJECTOR:
                    statusText = "Water Injector";
                    break;
                case RigWellResultFrame::UNDEFINED_PRODUCTION_TYPE:
                    break;
                default:
                    break;
                }
            }
        }
    }

    return statusText;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RimWellAllocationPlot::viewWidget()
{
    return m_wellAllocationPlotWidget;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::zoomAll()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlot* RimWellAllocationPlot::accumulatedWellFlowPlot()
{
    return m_accumulatedWellFlowPlot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimTotalWellAllocationPlot* RimWellAllocationPlot::totalWellFlowPlot()
{
    return m_totalWellAllocationPlot();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellAllocationPlot::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_wellName)
    {
        std::set<QString> sortedWellNames;
        if ( m_case && m_case->reservoirData() )
        {
            const cvf::Collection<RigSingleWellResultsData>& wellRes = m_case->reservoirData()->wellResults();

            for ( size_t wIdx = 0; wIdx < wellRes.size(); ++wIdx )
            {
                sortedWellNames.insert(wellRes[wIdx]->m_wellName);
            }
        }

        QIcon simWellIcon(":/Well.png");
        for ( const QString& wname: sortedWellNames )
        {
            options.push_back(caf::PdmOptionItemInfo(wname, wname, false, simWellIcon));
        }

        if (options.size() == 0)
        {
            options.push_front(caf::PdmOptionItemInfo("None", nullptr));
        }
    }
    else if (fieldNeedingOptions == &m_timeStep)
    {
        QStringList timeStepNames;

        if (m_case && m_case->reservoirData())
        {
            timeStepNames = m_case->timeStepStrings();
        }

        for (int i = 0; i < timeStepNames.size(); i++)
        {
            options.push_back(caf::PdmOptionItemInfo(timeStepNames[i], i));
        }
    }
    else if (fieldNeedingOptions == &m_case)
    {
        RimProject* proj = nullptr;
        this->firstAncestorOrThisOfType(proj);
        if (proj)
        {
            std::vector<RimEclipseResultCase*> cases;
            proj->descendantsIncludingThisOfType(cases);

            for (RimEclipseResultCase* c : cases)
            {
                options.push_back(caf::PdmOptionItemInfo(c->caseUserDescription(), c, false, c->uiIcon()));
            }
        }
    }
    else if (fieldNeedingOptions == &m_flowDiagSolution)
    {
        if (m_case)
        {
            std::vector<RimFlowDiagSolution*> flowSols = m_case->flowDiagSolutions();

            for (RimFlowDiagSolution* flowSol : flowSols)
            {
                options.push_back(caf::PdmOptionItemInfo(flowSol->userDescription(), flowSol, false, flowSol->uiIcon()));
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellAllocationPlot::wellName() const
{
    return m_wellName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::removeFromMdiAreaAndDeleteViewWidget()
{
    removeMdiWindowFromMdiArea();
    deleteViewWidget();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimViewWindow::fieldChangedByUi(changedField, oldValue, newValue);

    if (changedField == &m_userName ||
        changedField == &m_showPlotTitle)
    {
        updateWidgetTitleWindowTitle();
    }
    else if (   changedField == &m_wellName
             || changedField == &m_case
             || changedField == &m_timeStep)
    {
        updateFromWell();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QImage RimWellAllocationPlot::snapshotWindowContent()
{
    QImage image;

    // TODO

    return image;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::setDescription(const QString& description)
{
    m_userName = description;

    updateWidgetTitleWindowTitle();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellAllocationPlot::description() const
{
    return m_userName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::loadDataAndUpdate()
{
    updateMdiWindowVisibility();
    updateFromWell();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RimWellAllocationPlot::createViewWidget(QWidget* mainWindowParent)
{
    m_wellAllocationPlotWidget = new RiuWellAllocationPlot(this, mainWindowParent);
    return m_wellAllocationPlotWidget;
}


