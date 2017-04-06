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

#include "RigAccWellFlowCalculator.h"
#include "RigEclipseCaseData.h"
#include "RigFlowDiagResultAddress.h"
#include "RigFlowDiagResults.h"
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
#include "RimProject.h"
#include "RimTotalWellAllocationPlot.h"
#include "RimWellFlowRateCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"
#include "RimWellAllocationPlotLegend.h"

#include "RiuMainPlotWindow.h"
#include "RiuWellAllocationPlot.h"
#include "RiuWellLogTrack.h"

CAF_PDM_SOURCE_INIT(RimWellAllocationPlot, "WellAllocationPlot");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

namespace caf
{

template<>
void AppEnum<RimWellAllocationPlot::FlowType>::setUp()
{
    addItem(RimWellAllocationPlot::ACCUMULATED, "ACCUMULATED", "Accumulated");
    addItem(RimWellAllocationPlot::INFLOW, "INFLOW", "Inflow Rates");
    setDefault(RimWellAllocationPlot::ACCUMULATED);

}
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellAllocationPlot::RimWellAllocationPlot()
{
    CAF_PDM_InitObject("Well Allocation Plot", ":/WellAllocPlot16x16.png", "", "");

    CAF_PDM_InitField(&m_userName, "PlotDescription", QString("Flow Diagnostics Plot"), "Name", "", "", "");
    m_userName.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&m_showPlotTitle, "ShowPlotTitle", true, "Show Plot Title", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_case, "CurveCase", "Case", "", "", "");
    m_case.uiCapability()->setUiTreeChildrenHidden(true);

    CAF_PDM_InitField(&m_timeStep,                         "PlotTimeStep",                0,               "Time Step",                 "", "", "");
    CAF_PDM_InitField(&m_wellName,                         "WellName",                    QString("None"), "Well",                      "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_flowDiagSolution,        "FlowDiagSolution",                             "Plot Type",                 "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_flowType,                "FlowType",                                     "Flow Type",                 "", "", "");
    CAF_PDM_InitField(&m_groupSmallContributions,          "GroupSmallContributions",     true,            "Group Small Contributions", "", "", "");
    CAF_PDM_InitField(&m_smallContributionsThreshold,      "SmallContributionsThreshold", 0.005,           "Threshold",                 "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_accumulatedWellFlowPlot, "AccumulatedWellFlowPlot",                      "Accumulated Well Flow",     "", "", "");
    m_accumulatedWellFlowPlot.uiCapability()->setUiHidden(true);
    m_accumulatedWellFlowPlot = new RimWellLogPlot;
    m_accumulatedWellFlowPlot->setDepthUnit(RimDefines::UNIT_NONE);
    m_accumulatedWellFlowPlot->setDepthType(RimWellLogPlot::CONNECTION_NUMBER);
    m_accumulatedWellFlowPlot->setTrackLegendsVisible(false);
    m_accumulatedWellFlowPlot->uiCapability()->setUiIcon(QIcon(":/WellFlowPlot16x16.png"));

    CAF_PDM_InitFieldNoDefault(&m_totalWellAllocationPlot, "TotalWellFlowPlot", "Total Well Flow", "", "", "");
    m_totalWellAllocationPlot.uiCapability()->setUiHidden(true);
    m_totalWellAllocationPlot = new RimTotalWellAllocationPlot;

    CAF_PDM_InitFieldNoDefault(&m_wellAllocationPlotLegend, "WellAllocLegend", "Legend", "", "", "");
    m_wellAllocationPlotLegend.uiCapability()->setUiHidden(true);
    m_wellAllocationPlotLegend = new RimWellAllocationPlotLegend;

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

    // Use the active flow diag solutions, or the first one as default
    m_flowDiagSolution = eclView->cellResult()->flowDiagSolution();
    if ( !m_flowDiagSolution )
    {
        m_flowDiagSolution = m_case->defaultFlowDiagSolution();
    }

    loadDataAndUpdate();
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

    QString description;
    if (m_flowType() == ACCUMULATED)  description = "Accumulated Flow";
    if (m_flowType() == INFLOW)  description = "Inflow Rates";

    accumulatedWellFlowPlot()->setDescription(description + " (" + m_wellName + ")");

    if (!m_case) return;

    const RigSingleWellResultsData* wellResults = m_case->eclipseCaseData()->findWellResult(m_wellName);

    if (!wellResults) return;
   
   // Set up the Accumulated Well Flow Calculator

    std::vector< std::vector <cvf::Vec3d> > pipeBranchesCLCoords;
    std::vector< std::vector <RigWellResultPoint> > pipeBranchesCellIds;

    RigSimulationWellCenterLineCalculator::calculateWellPipeCenterlineFromWellFrame(m_case->eclipseCaseData(),
                                                                                    wellResults,
                                                                                    m_timeStep,
                                                                                    true,
                                                                                    true,
                                                                                    pipeBranchesCLCoords,
                                                                                    pipeBranchesCellIds);

    std::map<QString, const std::vector<double>* > tracerFractionCellValues = findRelevantTracerCellFractions(wellResults);

    std::unique_ptr< RigAccWellFlowCalculator > wfCalculator;

    double smallContributionThreshold = 0.0;
    if (m_groupSmallContributions()) smallContributionThreshold = m_smallContributionsThreshold;

    if ( tracerFractionCellValues.size() )
    {
        bool isProducer = (    wellResults->wellProductionType(m_timeStep) == RigWellResultFrame::PRODUCER 
                            || wellResults->wellProductionType(m_timeStep) == RigWellResultFrame::UNDEFINED_PRODUCTION_TYPE );
        RigEclCellIndexCalculator cellIdxCalc(m_case->eclipseCaseData()->mainGrid(), m_case->eclipseCaseData()->activeCellInfo(RifReaderInterface::MATRIX_RESULTS));
        wfCalculator.reset(new RigAccWellFlowCalculator(pipeBranchesCLCoords,
                                                        pipeBranchesCellIds,
                                                        tracerFractionCellValues,
                                                        cellIdxCalc, 
                                                        smallContributionThreshold, 
                                                        isProducer));
    }
    else 
    {
        if (pipeBranchesCLCoords.size() > 0)
        {
            wfCalculator.reset(new RigAccWellFlowCalculator(pipeBranchesCLCoords,
                                                            pipeBranchesCellIds,
                                                            smallContributionThreshold));
        }
    }

    auto depthType = accumulatedWellFlowPlot()->depthType();

    if (   depthType == RimWellLogPlot::MEASURED_DEPTH ) return;

    // Create tracks and curves from the calculated data

    size_t branchCount = pipeBranchesCLCoords.size();
    for (size_t brIdx = 0; brIdx < branchCount; ++brIdx)
    {
        // Skip Tiny dummy branches
        if (pipeBranchesCellIds[brIdx].size() <= 3) continue;

        RimWellLogTrack* plotTrack = new RimWellLogTrack();

        plotTrack->setDescription(QString("Branch %1").arg(brIdx + 1));

        accumulatedWellFlowPlot()->addTrack(plotTrack);

        const std::vector<double>& depthValues = depthType == RimWellLogPlot::CONNECTION_NUMBER ? wfCalculator->connectionNumbersFromTop(brIdx) :
                                                 depthType == RimWellLogPlot::PSEUDO_LENGTH ? wfCalculator->pseudoLengthFromTop(brIdx) :
                                                 depthType == RimWellLogPlot::TRUE_VERTICAL_DEPTH ? wfCalculator->trueVerticalDepth(brIdx) :
                                                 std::vector<double>();
        
        if ( m_flowDiagSolution )
        {
            std::vector<QString> tracerNames = wfCalculator->tracerNames();
            for (const QString& tracerName: tracerNames)
            {
                const std::vector<double>* accFlow = nullptr;
                if (depthType == RimWellLogPlot::CONNECTION_NUMBER)
                {
                    accFlow = &(m_flowType == ACCUMULATED ?
                                wfCalculator->accumulatedTracerFlowPrConnection(tracerName, brIdx):
                                wfCalculator->tracerFlowPrConnection(tracerName, brIdx));
                }
                else if ( depthType == RimWellLogPlot::PSEUDO_LENGTH || depthType == RimWellLogPlot::TRUE_VERTICAL_DEPTH)
                {
                    accFlow = &(m_flowType == ACCUMULATED ?
                                wfCalculator->accumulatedTracerFlowPrPseudoLength(tracerName, brIdx):
                                wfCalculator->tracerFlowPrPseudoLength(tracerName, brIdx));
                }

                addStackedCurve(tracerName, depthValues, *accFlow, plotTrack);
                //TODO: THIs is the data to be plotted...
            }
        }
        else
        {
            const std::vector<double>* accFlow = nullptr;
            if (depthType == RimWellLogPlot::CONNECTION_NUMBER)
            {
                accFlow = &(m_flowType == ACCUMULATED ?
                            wfCalculator->accumulatedFlowPrConnection(brIdx):
                            wfCalculator->flowPrConnection( brIdx));
            }
            else if ( depthType == RimWellLogPlot::PSEUDO_LENGTH || depthType == RimWellLogPlot::TRUE_VERTICAL_DEPTH)
            {
                accFlow = &(m_flowType == ACCUMULATED ?
                            wfCalculator->accumulatedFlowPrPseudoLength(brIdx):
                            wfCalculator->flowPrPseudoLength( brIdx));
            }

            addStackedCurve("Total", depthValues, *accFlow, plotTrack);
        }

        updateWellFlowPlotXAxisTitle(plotTrack);

    }

    QString wellStatusText = QString("(%1)").arg(RimWellAllocationPlot::wellStatusTextForTimeStep(m_wellName, m_case, m_timeStep));
    
    QString flowTypeText = m_flowDiagSolution() ? "Well Allocation": "Well Flow";
    setDescription(flowTypeText + ": " + m_wellName + " " + wellStatusText + ", " +  m_case->timeStepStrings()[m_timeStep] + " (" + m_case->caseUserDescription() + ")");
 
    /// Pie chart

    m_totalWellAllocationPlot->clearSlices();
    if (m_wellAllocationPlotWidget) m_wellAllocationPlotWidget->clearLegend();

    if (wfCalculator)
    {
        std::vector<std::pair<QString, double> > totalTracerFractions = wfCalculator->totalTracerFractions() ;

        for ( const auto& tracerVal : totalTracerFractions )
        {
            cvf::Color3f color;
            if ( m_flowDiagSolution )
                color = m_flowDiagSolution->tracerColor(tracerVal.first);
            else
                color = cvf::Color3f::DARK_GRAY;

            double tracerPercent = 100*tracerVal.second;

            m_totalWellAllocationPlot->addSlice(tracerVal.first, color, tracerPercent);
            if ( m_wellAllocationPlotWidget ) m_wellAllocationPlotWidget->addLegendItem(tracerVal.first, color, tracerPercent);
        }
    }

    if (m_wellAllocationPlotWidget) m_wellAllocationPlotWidget->showLegend(m_wellAllocationPlotLegend->isShowingLegend());
    m_totalWellAllocationPlot->updateConnectedEditors();

    accumulatedWellFlowPlot()->updateConnectedEditors();
    if (m_wellAllocationPlotWidget) m_wellAllocationPlotWidget->updateGeometry();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<QString, const std::vector<double> *> RimWellAllocationPlot::findRelevantTracerCellFractions(const RigSingleWellResultsData* wellResults)
{
    std::map<QString, const std::vector<double> *> tracerCellFractionValues;

    if ( m_flowDiagSolution && wellResults->hasWellResult(m_timeStep) )
    {
        RimFlowDiagSolution::TracerStatusType requestedTracerType = RimFlowDiagSolution::UNDEFINED;

        const RigWellResultFrame::WellProductionType prodType = wellResults->wellProductionType(m_timeStep);
        if (    prodType == RigWellResultFrame::PRODUCER 
             || prodType == RigWellResultFrame::UNDEFINED_PRODUCTION_TYPE )
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
            if ( m_flowDiagSolution->tracerStatusInTimeStep(tracerName, m_timeStep) == requestedTracerType )
            {
                RigFlowDiagResultAddress resAddr(RIG_FLD_CELL_FRACTION_RESNAME, tracerName.toStdString());
                const std::vector<double>* tracerCellFractions = m_flowDiagSolution->flowDiagResults()->resultValues(resAddr, m_timeStep);
                if (tracerCellFractions) tracerCellFractionValues[tracerName] = tracerCellFractions;
            }
        }
    }

    return tracerCellFractionValues;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::updateWellFlowPlotXAxisTitle(RimWellLogTrack* plotTrack)
{
    RigEclipseCaseData::UnitsType unitSet = m_case->eclipseCaseData()->unitsType();
    QString unitText;
    switch ( unitSet )
    {
        case RigEclipseCaseData::UNITS_METRIC:
        unitText = "[m^3/day]";
        break;
        case RigEclipseCaseData::UNITS_FIELD:
        unitText = "[Brl/day]";
        break;
        case RigEclipseCaseData::UNITS_LAB:
        unitText = "[cm^3/hr]";
        break;
        default:
        break;

    }

    plotTrack->setXAxisTitle("Flow Rate " + unitText);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::addStackedCurve(const QString& tracerName, 
                                            const std::vector<double>& depthValues, 
                                            const std::vector<double>& accFlow, 
                                            RimWellLogTrack* plotTrack)
{
    RimWellFlowRateCurve* curve = new RimWellFlowRateCurve;
    curve->setFlowValuesPrDepthValue(tracerName, depthValues, accFlow);

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
        const RigSingleWellResultsData* wellResults = eclipseResultCase->eclipseCaseData()->findWellResult(wellName);

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
    m_accumulatedWellFlowPlot()->zoomAll();
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
RimFlowDiagSolution* RimWellAllocationPlot::flowDiagSolution()
{
    return m_flowDiagSolution();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RimWellAllocationPlot::timeStep()
{
    return m_timeStep();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellAllocationPlot::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_wellName)
    {
        std::set<QString> sortedWellNames = this->findSortedWellNames();

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

        if (m_case && m_case->eclipseCaseData())
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
            //std::vector<RimFlowDiagSolution*> flowSols = m_case->flowDiagSolutions();
            // options.push_back(caf::PdmOptionItemInfo("None", nullptr));
            //for (RimFlowDiagSolution* flowSol : flowSols)
            //{
            //    options.push_back(caf::PdmOptionItemInfo(flowSol->userDescription(), flowSol, false, flowSol->uiIcon()));
            //}

            RimFlowDiagSolution* defaultFlowSolution =  m_case->defaultFlowDiagSolution();
            options.push_back(caf::PdmOptionItemInfo("Well Flow", nullptr));
            if (defaultFlowSolution)
            {
                options.push_back(caf::PdmOptionItemInfo("Allocation", defaultFlowSolution ));
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
void RimWellAllocationPlot::showPlotLegend(bool doShow)
{
    if (m_wellAllocationPlotWidget) m_wellAllocationPlotWidget->showLegend(doShow);
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
    else if ( changedField == &m_case)
    {
        if ( m_flowDiagSolution && m_case )
        {
            m_flowDiagSolution = m_case->defaultFlowDiagSolution();
        }
        else
        {
            m_flowDiagSolution = nullptr;
        }

        if (!m_case) m_timeStep = 0;
        else if (m_timeStep >= m_case->timeStepDates().size())
        {
            m_timeStep =  std::max(0, ((int)m_case->timeStepDates().size()) - 1);
        }

        std::set<QString> sortedWellNames = findSortedWellNames();
        if (!sortedWellNames.size()) m_wellName = "";
        else if ( sortedWellNames.count(m_wellName()) == 0 ){ m_wellName = *sortedWellNames.begin();}

        loadDataAndUpdate();
    }
    else if (   changedField == &m_wellName
             || changedField == &m_timeStep
             || changedField == &m_flowDiagSolution
             || changedField == &m_groupSmallContributions
             || changedField == &m_smallContributionsThreshold
             || changedField == &m_flowType )
    {
        loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<QString> RimWellAllocationPlot::findSortedWellNames()
{
    std::set<QString> sortedWellNames;
    if ( m_case && m_case->eclipseCaseData() )
    {
        const cvf::Collection<RigSingleWellResultsData>& wellRes = m_case->eclipseCaseData()->wellResults();

        for ( size_t wIdx = 0; wIdx < wellRes.size(); ++wIdx )
        {
            sortedWellNames.insert(wellRes[wIdx]->m_wellName);
        }
    }

    return sortedWellNames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QImage RimWellAllocationPlot::snapshotWindowContent()
{
    QImage image;

    if (m_wellAllocationPlotWidget)
    {
        QPixmap pix = QPixmap::grabWidget(m_wellAllocationPlotWidget);
        image = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    m_smallContributionsThreshold.uiCapability()->setUiReadOnly(!m_groupSmallContributions());
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
    m_accumulatedWellFlowPlot->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* RimWellAllocationPlot::createViewWidget(QWidget* mainWindowParent)
{
    m_wellAllocationPlotWidget = new RiuWellAllocationPlot(this, mainWindowParent);
    return m_wellAllocationPlotWidget;
}


