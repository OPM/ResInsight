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

#include "RiaPreferences.h"

#include "RigAccWellFlowCalculator.h"
#include "RigEclipseCaseData.h"
#include "RigFlowDiagResultAddress.h"
#include "RigFlowDiagResults.h"
#include "RigSimWellData.h"
#include "RigSimulationWellCenterLineCalculator.h"
#include "RigSimulationWellCoordsAndMD.h"
#include "RigWellResultPoint.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimFlowDiagSolution.h"
#include "RimProject.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"
#include "RimTofAccumulatedPhaseFractionsPlot.h"
#include "RimTools.h"
#include "RimTotalWellAllocationPlot.h"
#include "RimWellAllocationPlotLegend.h"
#include "RimWellFlowRateCurve.h"
#include "RimWellLogCurveCommonDataSource.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"

#include "RimWellLogFile.h"
#include "RimWellPlotTools.h"
#include "RiuPlotMainWindow.h"
#include "RiuQwtPlotWidget.h"
#include "RiuWellAllocationPlot.h"

CAF_PDM_SOURCE_INIT( RimWellAllocationPlot, "WellAllocationPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

namespace caf
{
template <>
void AppEnum<RimWellAllocationPlot::FlowType>::setUp()
{
    addItem( RimWellAllocationPlot::ACCUMULATED, "ACCUMULATED", "Accumulated" );
    addItem( RimWellAllocationPlot::INFLOW, "INFLOW", "Inflow Rates" );
    setDefault( RimWellAllocationPlot::ACCUMULATED );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellAllocationPlot::RimWellAllocationPlot()
{
    CAF_PDM_InitObject( "Well Allocation Plot", ":/WellAllocPlot16x16.png", "", "" );

    CAF_PDM_InitField( &m_userName, "PlotDescription", QString( "Flow Diagnostics Plot" ), "Name" );
    m_userName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_showPlotTitle, "ShowPlotTitle", true, "Show Plot Title" );

    CAF_PDM_InitField( &m_branchDetection,
                       "BranchDetection",
                       true,
                       "Branch Detection",
                       "",
                       "Compute branches based on how simulation well cells are organized",
                       "" );

    CAF_PDM_InitFieldNoDefault( &m_case, "CurveCase", "Case" );
    m_case.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitField( &m_timeStep, "PlotTimeStep", 0, "Time Step" );
    CAF_PDM_InitField( &m_wellName, "WellName", QString( "None" ), "Well" );
    CAF_PDM_InitFieldNoDefault( &m_flowDiagSolution, "FlowDiagSolution", "Plot Type" );
    CAF_PDM_InitFieldNoDefault( &m_flowType, "FlowType", "Flow Type" );
    CAF_PDM_InitField( &m_groupSmallContributions, "GroupSmallContributions", true, "Group Small Contributions" );
    CAF_PDM_InitField( &m_smallContributionsThreshold, "SmallContributionsThreshold", 0.005, "Threshold" );
    CAF_PDM_InitFieldNoDefault( &m_accumulatedWellFlowPlot, "AccumulatedWellFlowPlot", "Accumulated Well Flow" );
    m_accumulatedWellFlowPlot.uiCapability()->setUiTreeHidden( true );
    m_accumulatedWellFlowPlot = new RimWellLogPlot;
    m_accumulatedWellFlowPlot->setDepthUnit( RiaDefines::DepthUnitType::UNIT_NONE );
    m_accumulatedWellFlowPlot->setDepthType( RiaDefines::DepthTypeEnum::CONNECTION_NUMBER );
    m_accumulatedWellFlowPlot->setLegendsVisible( false );
    m_accumulatedWellFlowPlot->uiCapability()->setUiIconFromResourceString( ":/WellFlowPlot16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_totalWellAllocationPlot, "TotalWellFlowPlot", "Total Well Flow" );
    m_totalWellAllocationPlot.uiCapability()->setUiTreeHidden( true );
    m_totalWellAllocationPlot = new RimTotalWellAllocationPlot;

    CAF_PDM_InitFieldNoDefault( &m_wellAllocationPlotLegend, "WellAllocLegend", "Legend" );
    m_wellAllocationPlotLegend.uiCapability()->setUiTreeHidden( true );
    m_wellAllocationPlotLegend = new RimWellAllocationPlotLegend;

    CAF_PDM_InitFieldNoDefault( &m_tofAccumulatedPhaseFractionsPlot,
                                "TofAccumulatedPhaseFractionsPlot",
                                "TOF Accumulated Phase Fractions");
    m_tofAccumulatedPhaseFractionsPlot.uiCapability()->setUiTreeHidden( true );
    m_tofAccumulatedPhaseFractionsPlot = new RimTofAccumulatedPhaseFractionsPlot;

    this->setAsPlotMdiWindow();

    m_accumulatedWellFlowPlot->setAvailableDepthUnits( {} );
    m_accumulatedWellFlowPlot->setAvailableDepthTypes( { RiaDefines::DepthTypeEnum::CONNECTION_NUMBER,
                                                         RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH,
                                                         RiaDefines::DepthTypeEnum::PSEUDO_LENGTH } );

    m_accumulatedWellFlowPlot->setCommonDataSourceEnabled( false );

    m_showWindow = false;
    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellAllocationPlot::~RimWellAllocationPlot()
{
    removeMdiWindowFromMdiArea();

    delete m_accumulatedWellFlowPlot();
    delete m_totalWellAllocationPlot();
    delete m_tofAccumulatedPhaseFractionsPlot();

    if ( m_wellAllocationPlotWidget )
    {
        m_wellAllocationPlotWidget->hide();
        m_wellAllocationPlotWidget->setParent( nullptr );
        delete m_wellAllocationPlotWidget;
        m_wellAllocationPlotWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
/// TODO: implement properly
//--------------------------------------------------------------------------------------------------
int RimWellAllocationPlot::id() const
{
    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::setFromSimulationWell( RimSimWellInView* simWell )
{
    m_showWindow = true;

    RimEclipseView* eclView;
    simWell->firstAncestorOrThisOfType( eclView );
    RimEclipseResultCase* eclCase;
    simWell->firstAncestorOrThisOfType( eclCase );

    m_case     = eclCase;
    m_wellName = simWell->simWellData()->m_wellName;
    m_timeStep = eclView->currentTimeStep();

    // Use the active flow diag solutions, or the first one as default
    m_flowDiagSolution = eclView->cellResult()->flowDiagSolution();
    if ( !m_flowDiagSolution )
    {
        m_flowDiagSolution = m_case->defaultFlowDiagSolution();
    }

    onLoadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::deleteViewWidget()
{
    if ( m_wellAllocationPlotWidget )
    {
        m_wellAllocationPlotWidget->hide();
        m_wellAllocationPlotWidget->setParent( nullptr );
        delete m_wellAllocationPlotWidget;
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
        accumulatedWellFlowPlot()->descendantsIncludingThisOfType( tracks );

        for ( RimWellLogTrack* t : tracks )
        {
            accumulatedWellFlowPlot()->removePlot( t );
            delete t;
        }
    }

    CVF_ASSERT( accumulatedWellFlowPlot()->plotCount() == 0 );

    QString description;
    if ( m_flowType() == ACCUMULATED ) description = "Accumulated Flow";
    if ( m_flowType() == INFLOW ) description = "Inflow Rates";

    RimWellLogPlotNameConfig* nameConfig = accumulatedWellFlowPlot()->nameConfig();
    nameConfig->setCustomName( description );
    nameConfig->setAutoNameTags( false, true, false, false, false );
    nameConfig->setFieldVisibility( true, true, true, false, false );
    accumulatedWellFlowPlot()->updateAutoName();

    if ( !m_case ) return;

    const RigSimWellData* simWellData = m_case->eclipseCaseData()->findSimWellData( m_wellName );

    if ( !simWellData ) return;

    // Set up the Accumulated Well Flow Calculator

    std::vector<std::vector<cvf::Vec3d>>         pipeBranchesCLCoords;
    std::vector<std::vector<RigWellResultPoint>> pipeBranchesCellIds;

    RigSimulationWellCenterLineCalculator::calculateWellPipeCenterlineFromWellFrame( m_case->eclipseCaseData(),
                                                                                     simWellData,
                                                                                     m_timeStep,
                                                                                     m_branchDetection,
                                                                                     true,
                                                                                     pipeBranchesCLCoords,
                                                                                     pipeBranchesCellIds );

    std::map<QString, const std::vector<double>*> tracerFractionCellValues = findRelevantTracerCellFractions( simWellData );

    std::unique_ptr<RigAccWellFlowCalculator> wfCalculator;

    double smallContributionThreshold = 0.0;
    if ( m_groupSmallContributions() ) smallContributionThreshold = m_smallContributionsThreshold;

    if ( tracerFractionCellValues.size() )
    {
        bool isProducer = ( simWellData->wellProductionType( m_timeStep ) == RiaDefines::WellProductionType::PRODUCER ||
                            simWellData->wellProductionType( m_timeStep ) ==
                                RiaDefines::WellProductionType::UNDEFINED_PRODUCTION_TYPE );
        RigEclCellIndexCalculator cellIdxCalc( m_case->eclipseCaseData()->mainGrid(),
                                               m_case->eclipseCaseData()->activeCellInfo(
                                                   RiaDefines::PorosityModelType::MATRIX_MODEL ) );
        wfCalculator.reset( new RigAccWellFlowCalculator( pipeBranchesCLCoords,
                                                          pipeBranchesCellIds,
                                                          tracerFractionCellValues,
                                                          cellIdxCalc,
                                                          smallContributionThreshold,
                                                          isProducer ) );
    }
    else
    {
        if ( pipeBranchesCLCoords.size() > 0 )
        {
            wfCalculator.reset(
                new RigAccWellFlowCalculator( pipeBranchesCLCoords, pipeBranchesCellIds, smallContributionThreshold ) );
        }
    }

    auto depthType = accumulatedWellFlowPlot()->depthType();

    if ( depthType == RiaDefines::DepthTypeEnum::MEASURED_DEPTH ) return;

    // Create tracks and curves from the calculated data

    size_t branchCount = pipeBranchesCLCoords.size();
    for ( size_t brIdx = 0; brIdx < branchCount; ++brIdx )
    {
        // Skip Tiny dummy branches
        if ( pipeBranchesCellIds[brIdx].size() <= 3 ) continue;

        RimWellLogTrack* plotTrack = new RimWellLogTrack();

        plotTrack->setDescription( QString( "Branch %1" ).arg( brIdx + 1 ) );
        plotTrack->setFormationsForCaseWithSimWellOnly( true );
        plotTrack->setFormationBranchIndex( (int)brIdx );

        accumulatedWellFlowPlot()->addPlot( plotTrack );

        const std::vector<double>& depthValues = depthType == RiaDefines::DepthTypeEnum::CONNECTION_NUMBER
                                                     ? wfCalculator->connectionNumbersFromTop( brIdx )
                                                     : depthType == RiaDefines::DepthTypeEnum::PSEUDO_LENGTH
                                                           ? wfCalculator->pseudoLengthFromTop( brIdx )
                                                           : depthType == RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH
                                                                 ? wfCalculator->trueVerticalDepth( brIdx )
                                                                 : std::vector<double>();

        if ( !depthValues.empty() )
        {
            std::vector<QString> tracerNames = wfCalculator->tracerNames();
            for ( const QString& tracerName : tracerNames )
            {
                std::vector<double> curveDepthValues = depthValues;
                std::vector<double> accFlow;
                if ( depthType == RiaDefines::DepthTypeEnum::CONNECTION_NUMBER )
                {
                    accFlow = ( m_flowType == ACCUMULATED
                                    ? wfCalculator->accumulatedTracerFlowPrConnection( tracerName, brIdx )
                                    : wfCalculator->tracerFlowPrConnection( tracerName, brIdx ) );

                    // Insert the first depth position again, to add a <maxdepth, 0.0> value pair
                    curveDepthValues.insert( curveDepthValues.begin(), curveDepthValues[0] );
                    accFlow.insert( accFlow.begin(), 0.0 );

                    if ( m_flowType == ACCUMULATED && brIdx == 0 && !accFlow.empty() ) // Add fictitious point to -1 for
                                                                                       // first branch
                    {
                        accFlow.push_back( accFlow.back() );
                        curveDepthValues.push_back( -1.0 );
                    }

                    // Shift the "bars" to make connection number tick at the midpoint of the constant value
                    // when showing in flow rate
                    if ( m_flowType == INFLOW )
                    {
                        for ( double& connNum : curveDepthValues )
                        {
                            connNum += 0.5;
                        }
                    }
                }
                else if ( depthType == RiaDefines::DepthTypeEnum::PSEUDO_LENGTH ||
                          depthType == RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH )
                {
                    accFlow = ( m_flowType == ACCUMULATED
                                    ? wfCalculator->accumulatedTracerFlowPrPseudoLength( tracerName, brIdx )
                                    : wfCalculator->tracerFlowPrPseudoLength( tracerName, brIdx ) );

                    // Insert the first depth position again, to add a <maxdepth, 0.0> value pair
                    curveDepthValues.insert( curveDepthValues.begin(), curveDepthValues[0] );
                    accFlow.insert( accFlow.begin(), 0.0 );

                    if ( brIdx == 0 && branchCount > 1 )
                    {
                        // Add a dummy negative depth value to make the contribution
                        // from other branches connected to well head visible

                        auto   minmax_it = std::minmax_element( curveDepthValues.begin(), curveDepthValues.end() );
                        double availableMinDepth = *( minmax_it.first );
                        double availableMaxDepth = *( minmax_it.second );

                        double depthSpan = 0.1 * cvf::Math::abs( availableMinDepth - availableMaxDepth );

                        // Round off value to floored decade
                        double logDecValue = log10( depthSpan );
                        logDecValue        = cvf::Math::floor( logDecValue );
                        depthSpan          = pow( 10.0, logDecValue );

                        double dummyNegativeDepthValue = curveDepthValues.back() - depthSpan;

                        curveDepthValues.push_back( dummyNegativeDepthValue );
                        accFlow.push_back( accFlow.back() );
                    }
                }

                if ( !accFlow.empty() )
                {
                    addStackedCurve( tracerName, depthType, curveDepthValues, accFlow, plotTrack );
                    // TODO: THIs is the data to be plotted...
                }
            }
        }

        updateWellFlowPlotXAxisTitle( plotTrack );
    }

    QString wellStatusText =
        QString( "(%1)" ).arg( RimWellAllocationPlot::wellStatusTextForTimeStep( m_wellName, m_case, m_timeStep ) );

    QString flowTypeText = m_flowDiagSolution() ? "Well Allocation" : "Well Flow";
    setDescription( flowTypeText + ": " + m_wellName + " " + wellStatusText + ", " +
                    m_case->timeStepStrings()[m_timeStep] + " (" + m_case->caseUserDescription() + ")" );

    /// Pie chart

    m_totalWellAllocationPlot->clearSlices();
    if ( m_wellAllocationPlotWidget ) m_wellAllocationPlotWidget->clearLegend();

    if ( wfCalculator )
    {
        std::vector<std::pair<QString, double>> totalTracerFractions = wfCalculator->totalTracerFractions();

        for ( const auto& tracerVal : totalTracerFractions )
        {
            cvf::Color3f color;
            if ( m_flowDiagSolution )
                color = m_flowDiagSolution->tracerColor( tracerVal.first );
            else
                color = getTracerColor( tracerVal.first );

            double tracerPercent = 100 * tracerVal.second;

            m_totalWellAllocationPlot->addSlice( tracerVal.first, color, tracerPercent );
            if ( m_wellAllocationPlotWidget )
                m_wellAllocationPlotWidget->addLegendItem( tracerVal.first, color, tracerPercent );
        }
    }

    if ( m_wellAllocationPlotWidget )
        m_wellAllocationPlotWidget->showLegend( m_wellAllocationPlotLegend->isShowingLegend() );
    m_totalWellAllocationPlot->updateConnectedEditors();

    accumulatedWellFlowPlot()->updateConnectedEditors();
    m_tofAccumulatedPhaseFractionsPlot->reloadFromWell();
    m_tofAccumulatedPhaseFractionsPlot->updateConnectedEditors();

    if ( m_wellAllocationPlotWidget ) m_wellAllocationPlotWidget->updateGeometry();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, const std::vector<double>*>
    RimWellAllocationPlot::findRelevantTracerCellFractions( const RigSimWellData* simWellData )
{
    std::map<QString, const std::vector<double>*> tracerCellFractionValues;

    if ( m_flowDiagSolution && simWellData->hasWellResult( m_timeStep ) )
    {
        RimFlowDiagSolution::TracerStatusType requestedTracerType = RimFlowDiagSolution::TracerStatusType::UNDEFINED;

        const RiaDefines::WellProductionType prodType = simWellData->wellProductionType( m_timeStep );
        if ( prodType == RiaDefines::WellProductionType::PRODUCER ||
             prodType == RiaDefines::WellProductionType::UNDEFINED_PRODUCTION_TYPE )
        {
            requestedTracerType = RimFlowDiagSolution::TracerStatusType::INJECTOR;
        }
        else
        {
            requestedTracerType = RimFlowDiagSolution::TracerStatusType::PRODUCER;
        }

        std::vector<QString> tracerNames = m_flowDiagSolution->tracerNames();
        for ( const QString& tracerName : tracerNames )
        {
            if ( m_flowDiagSolution->tracerStatusInTimeStep( tracerName, m_timeStep ) == requestedTracerType )
            {
                RigFlowDiagResultAddress   resAddr( RIG_FLD_CELL_FRACTION_RESNAME,
                                                  RigFlowDiagResultAddress::PHASE_ALL,
                                                  tracerName.toStdString() );
                const std::vector<double>* tracerCellFractions =
                    m_flowDiagSolution->flowDiagResults()->resultValues( resAddr, m_timeStep );
                if ( tracerCellFractions ) tracerCellFractionValues[tracerName] = tracerCellFractions;
            }
        }
    }

    return tracerCellFractionValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::updateWellFlowPlotXAxisTitle( RimWellLogTrack* plotTrack )
{
    RiaDefines::EclipseUnitSystem     unitSet   = m_case->eclipseCaseData()->unitsType();
    RimWellLogFile::WellFlowCondition condition = m_flowDiagSolution ? RimWellLogFile::WELL_FLOW_COND_RESERVOIR
                                                                     : RimWellLogFile::WELL_FLOW_COND_STANDARD;

    QString axisTitle = RimWellPlotTools::flowPlotAxisTitle( condition, unitSet );
    plotTrack->setXAxisTitle( axisTitle );

#if 0
    if (m_flowDiagSolution) 
    {
        QString unitText;
        switch ( unitSet )
        {
            case RiaEclipseUnitTools::UNITS_METRIC:
            unitText = "[m<sup>3</sup>/day]";
            break;
            case RiaEclipseUnitTools::UNITS_FIELD:
            unitText = "[Brl/day]";
            break;
            case RiaEclipseUnitTools::UNITS_LAB:
            unitText = "[cm<sup>3</sup>/hr]";
            break;
            default:
            break;

        }
        plotTrack->setXAxisTitle("Reservoir Flow Rate " + unitText);
    }
    else
    {
        QString unitText;
        switch ( unitSet )
        {
            case RiaEclipseUnitTools::UNITS_METRIC:
            unitText = "[Liquid Sm<sup>3</sup>/day], [Gas kSm<sup>3</sup>/day]";
            break;
            case RiaEclipseUnitTools::UNITS_FIELD:
            unitText = "[Liquid BBL/day], [Gas BOE/day]";
            break;
            case RiaEclipseUnitTools::UNITS_LAB:
            unitText = "[cm<sup>3</sup>/hr]";
            break;
            default:
            break;

        }
        plotTrack->setXAxisTitle("Surface Flow Rate " + unitText);
    }
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::addStackedCurve( const QString&             tracerName,
                                             RiaDefines::DepthTypeEnum  depthType,
                                             const std::vector<double>& depthValues,
                                             const std::vector<double>& accFlow,
                                             RimWellLogTrack*           plotTrack )
{
    RimWellFlowRateCurve* curve = new RimWellFlowRateCurve;
    curve->setFlowValuesPrDepthValue( tracerName, depthType, depthValues, accFlow );

    if ( m_flowDiagSolution )
    {
        curve->setColor( m_flowDiagSolution->tracerColor( tracerName ) );
    }
    else
    {
        curve->setColor( getTracerColor( tracerName ) );
    }

    plotTrack->addCurve( curve );

    curve->loadDataAndUpdate( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::updateWidgetTitleWindowTitle()
{
    updateMdiWindowTitle();

    if ( m_wellAllocationPlotWidget )
    {
        if ( m_showPlotTitle )
        {
            m_wellAllocationPlotWidget->showTitle( m_userName );
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
QString RimWellAllocationPlot::wellStatusTextForTimeStep( const QString&              wellName,
                                                          const RimEclipseResultCase* eclipseResultCase,
                                                          size_t                      timeStep )
{
    QString statusText = "Undefined";

    if ( eclipseResultCase )
    {
        const RigSimWellData* simWellData = eclipseResultCase->eclipseCaseData()->findSimWellData( wellName );

        if ( simWellData )
        {
            if ( simWellData->hasWellResult( timeStep ) )
            {
                const RigWellResultFrame* wellResultFrame = simWellData->wellResultFrame( timeStep );

                RiaDefines::WellProductionType prodType = wellResultFrame->m_productionType;

                switch ( prodType )
                {
                    case RiaDefines::WellProductionType::PRODUCER:
                        statusText = "Producer";
                        break;
                    case RiaDefines::WellProductionType::OIL_INJECTOR:
                        statusText = "Oil Injector";
                        break;
                    case RiaDefines::WellProductionType::GAS_INJECTOR:
                        statusText = "Gas Injector";
                        break;
                    case RiaDefines::WellProductionType::WATER_INJECTOR:
                        statusText = "Water Injector";
                        break;
                    case RiaDefines::WellProductionType::UNDEFINED_PRODUCTION_TYPE:
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
/// TODO: Implement properly
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::assignIdIfNecessary()
{
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
RimTofAccumulatedPhaseFractionsPlot* RimWellAllocationPlot::tofAccumulatedPhaseFractionsPlot()
{
    return m_tofAccumulatedPhaseFractionsPlot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObject* RimWellAllocationPlot::plotLegend()
{
    return m_wellAllocationPlotLegend;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseResultCase* RimWellAllocationPlot::rimCase()
{
    return m_case();
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
RimWellAllocationPlot::FlowType RimWellAllocationPlot::flowType()
{
    return m_flowType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimWellAllocationPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_wellName )
    {
        std::set<QString> sortedWellNames = this->findSortedWellNames();

        caf::IconProvider simWellIcon( ":/Well.svg" );
        for ( const QString& wname : sortedWellNames )
        {
            options.push_back( caf::PdmOptionItemInfo( wname, wname, false, simWellIcon ) );
        }

        if ( options.size() == 0 )
        {
            options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
        }
    }
    else if ( fieldNeedingOptions == &m_timeStep )
    {
        RimTools::timeStepsForCase( m_case, &options );

        if ( options.size() == 0 )
        {
            options.push_front( caf::PdmOptionItemInfo( "None", -1 ) );
        }
    }
    else if ( fieldNeedingOptions == &m_case )
    {
        RimProject* proj = nullptr;
        this->firstAncestorOrThisOfType( proj );
        if ( proj )
        {
            std::vector<RimEclipseResultCase*> cases;
            proj->descendantsIncludingThisOfType( cases );

            for ( RimEclipseResultCase* c : cases )
            {
                options.push_back( caf::PdmOptionItemInfo( c->caseUserDescription(), c, false, c->uiIconProvider() ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_flowDiagSolution )
    {
        if ( m_case )
        {
            // std::vector<RimFlowDiagSolution*> flowSols = m_case->flowDiagSolutions();
            // options.push_back(caf::PdmOptionItemInfo("None", nullptr));
            // for (RimFlowDiagSolution* flowSol : flowSols)
            //{
            //    options.push_back(caf::PdmOptionItemInfo(flowSol->userDescription(), flowSol, false,
            //    flowSol->uiIcon()));
            //}

            RimFlowDiagSolution* defaultFlowSolution = m_case->defaultFlowDiagSolution();
            options.push_back( caf::PdmOptionItemInfo( "Well Flow", nullptr ) );
            if ( defaultFlowSolution )
            {
                options.push_back( caf::PdmOptionItemInfo( "Allocation", defaultFlowSolution ) );
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
void RimWellAllocationPlot::showPlotLegend( bool doShow )
{
    if ( m_wellAllocationPlotWidget ) m_wellAllocationPlotWidget->showLegend( doShow );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellAllocationPlot::fontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::updateFonts()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                              const QVariant&            oldValue,
                                              const QVariant&            newValue )
{
    RimViewWindow::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_userName || changedField == &m_showPlotTitle )
    {
        updateWidgetTitleWindowTitle();
    }
    else if ( changedField == &m_case )
    {
        if ( m_flowDiagSolution && m_case )
        {
            m_flowDiagSolution = m_case->defaultFlowDiagSolution();
        }
        else
        {
            m_flowDiagSolution = nullptr;
        }

        if ( !m_case )
            m_timeStep = 0;
        else if ( m_timeStep >= static_cast<int>( m_case->timeStepDates().size() ) )
        {
            m_timeStep = std::max( 0, ( (int)m_case->timeStepDates().size() ) - 1 );
        }

        std::set<QString> sortedWellNames = findSortedWellNames();
        if ( !sortedWellNames.size() )
            m_wellName = "";
        else if ( sortedWellNames.count( m_wellName() ) == 0 )
        {
            m_wellName = *sortedWellNames.begin();
        }

        onLoadDataAndUpdate();
    }
    else if ( changedField == &m_wellName || changedField == &m_timeStep || changedField == &m_flowDiagSolution ||
              changedField == &m_groupSmallContributions || changedField == &m_smallContributionsThreshold ||
              changedField == &m_flowType || changedField == &m_branchDetection )
    {
        onLoadDataAndUpdate();
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
        const cvf::Collection<RigSimWellData>& simWellData = m_case->eclipseCaseData()->wellResults();

        for ( size_t wIdx = 0; wIdx < simWellData.size(); ++wIdx )
        {
            sortedWellNames.insert( simWellData[wIdx]->m_wellName );
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

    if ( m_wellAllocationPlotWidget )
    {
        QPixmap pix = m_wellAllocationPlotWidget->grab();
        image       = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_userName );
    uiOrdering.add( &m_showPlotTitle );

    caf::PdmUiGroup& dataGroup = *uiOrdering.addNewGroup( "Plot Data" );
    dataGroup.add( &m_case );
    dataGroup.add( &m_timeStep );
    dataGroup.add( &m_wellName );
    dataGroup.add( &m_branchDetection );

    caf::PdmUiGroup& optionGroup = *uiOrdering.addNewGroup( "Options" );
    optionGroup.add( &m_flowDiagSolution );
    optionGroup.add( &m_flowType );
    optionGroup.add( &m_groupSmallContributions );
    optionGroup.add( &m_smallContributionsThreshold );
    m_smallContributionsThreshold.uiCapability()->setUiReadOnly( !m_groupSmallContributions() );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::setDescription( const QString& description )
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
void RimWellAllocationPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();

    if ( !m_case ) return;

    // If no 3D view is open, we have to make sure the case is opened
    if ( !m_case->ensureReservoirCaseIsOpen() )
    {
        return;
    }

    updateFromWell();
    m_accumulatedWellFlowPlot->loadDataAndUpdate();
    updateFormationNamesData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimWellAllocationPlot::createViewWidget( QWidget* mainWindowParent )
{
    m_wellAllocationPlotWidget = new RiuWellAllocationPlot( this, mainWindowParent );
    return m_wellAllocationPlotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimWellAllocationPlot::getTracerColor( const QString& tracerName )
{
    if ( tracerName == RIG_FLOW_OIL_NAME ) return cvf::Color3f::DARK_GREEN;
    if ( tracerName == RIG_FLOW_GAS_NAME ) return cvf::Color3f::DARK_RED;
    if ( tracerName == RIG_FLOW_WATER_NAME ) return cvf::Color3f::BLUE;
    return cvf::Color3f::DARK_GRAY;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellAllocationPlot::updateFormationNamesData() const
{
    for ( size_t i = 0; i < m_accumulatedWellFlowPlot->plotCount(); ++i )
    {
        RimWellLogTrack* track = dynamic_cast<RimWellLogTrack*>( m_accumulatedWellFlowPlot->plotByIndex( i ) );
        if ( track )
        {
            track->setAndUpdateSimWellFormationNamesData( m_case, m_wellName );
        }
    }
}
