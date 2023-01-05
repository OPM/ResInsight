/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RimHistoryWellAllocationPlot.h"

#include "RiaColorTools.h"
#include "RiaDefines.h"
#include "RiaQDateTimeTools.h"
#include "RiaTimeTTools.h"

#include "RigAccWellFlowCalculator.h"
#include "RigEclipseCaseData.h"
#include "RigSimWellData.h"
#include "RigSimulationWellCenterLineCalculator.h"
#include "RigWellResultPoint.h"

#include "RimEclipseCaseTools.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimFlowDiagSolution.h"
#include "RimHistoryWellFlowDataCollection.h"
#include "RimSimWellInView.h"
#include "RimStackablePlotCurve.h"
#include "RimWellAllocationTools.h"

#include "RiuContextMenuLauncher.h"
#include "RiuPlotCurve.h"
#include "RiuPlotWidget.h"
#include "RiuQwtCurvePointTracker.h"
#include "RiuQwtPlotTools.h"
#include "RiuQwtPlotWidget.h"

#include "cafCmdFeatureMenuBuilder.h"

#pragma optimize( "", off )

CAF_PDM_SOURCE_INIT( RimHistoryWellAllocationPlot, "RimHistoryWellAllocationPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
namespace caf
{
template <>
void AppEnum<RimHistoryWellAllocationPlot::FlowValueType>::setUp()
{
    addItem( RimHistoryWellAllocationPlot::FLOW_RATE, "FLOW_RATE", "Flow Rates" );
    addItem( RimHistoryWellAllocationPlot::FLOW_VOLUME, "FLOW_VOLUME", "Flow Volumes" );
    addItem( RimHistoryWellAllocationPlot::ACCUMULATED_FLOW_VOLUME, "ACCUMULATED_FLOW_VOLUME", "Accumulated Flow Volumes" );
    addItem( RimHistoryWellAllocationPlot::PERCENTAGE, "PERCENTAGE", "Percentage" );
    setDefault( RimHistoryWellAllocationPlot::FLOW_RATE );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistoryWellAllocationPlot::RimHistoryWellAllocationPlot()
{
    // TODO: Add icon
    CAF_PDM_InitObject( "History Well Allocation Plot", ":/HistoryWellAllocPlot16x16.png" );

    CAF_PDM_InitField( &m_userName, "PlotDescription", QString( "History Flow Diagnostics Plot" ), "Name" );
    m_userName.uiCapability()->setUiReadOnly( true );
    CAF_PDM_InitField( &m_branchDetection,
                       "BranchDetection",
                       true,
                       "Branch Detection",
                       "",
                       "Compute branches based on how simulation well cells are organized",
                       "" );
    CAF_PDM_InitFieldNoDefault( &m_case, "CurveCase", "Case" );
    m_case.uiCapability()->setUiTreeChildrenHidden( true );
    CAF_PDM_InitField( &m_wellName, "WellName", QString( "None" ), "Well" );

    CAF_PDM_InitFieldNoDefault( &m_flowDiagSolution, "FlowDiagSolution", "Plot Type" );
    CAF_PDM_InitFieldNoDefault( &m_flowValueType, "FlowValueType", "Value Type" );
    CAF_PDM_InitField( &m_groupSmallContributions, "GroupSmallContributions", true, "Group Small Contributions" );
    CAF_PDM_InitField( &m_smallContributionsThreshold, "SmallContributionsThreshold", 0.005, "Threshold" );

    setAsPlotMdiWindow();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistoryWellAllocationPlot::~RimHistoryWellAllocationPlot()
{
    removeMdiWindowFromMdiArea();
    deleteViewWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistoryWellAllocationPlot::setDescription( const QString& description )
{
    m_userName = description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistoryWellAllocationPlot::setFromSimulationWell( RimSimWellInView* simWell )
{
    m_showWindow = true;

    RimEclipseView* eclView;
    simWell->firstAncestorOrThisOfType( eclView );
    RimEclipseResultCase* eclCase;
    simWell->firstAncestorOrThisOfType( eclCase );

    m_case     = eclCase;
    m_wellName = simWell->simWellData()->m_wellName;

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
RiuPlotWidget* RimHistoryWellAllocationPlot::plotWidget()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimHistoryWellAllocationPlot::asciiDataForPlotExport() const
{
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimHistoryWellAllocationPlot::description() const
{
    return uiName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimHistoryWellAllocationPlot::viewWidget()
{
    return plotWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimHistoryWellAllocationPlot::snapshotWindowContent()
{
    QImage image;

    if ( m_plotWidget )
    {
        QPixmap pix = m_plotWidget->grab();
        image       = pix.toImage();
    }

    return image;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget* RimHistoryWellAllocationPlot::doCreatePlotViewWidget( QWidget* mainWindowParent )
{
    // If called multiple times?
    if ( m_plotWidget )
    {
        return m_plotWidget;
    }

    auto* plotWidget = new RiuQwtPlotWidget( this, mainWindowParent );
    new RiuQwtCurvePointTracker( plotWidget->qwtPlot(), true, nullptr );

    // Remove event filter to disable unwanted highlighting on left click in plot.
    plotWidget->removeEventFilter();

    /* caf::CmdFeatureMenuBuilder menuBuilder;
     menuBuilder << "RicShowPlotDataFeature";
     new RiuContextMenuLauncher( plotWidget, menuBuilder );*/

    m_plotWidget = plotWidget;
    m_plotWidget->setAxisTitleEnabled( RiuPlotAxis::defaultLeft(), true );
    /*RiuQwtPlotTools::enableDateBasedBottomXAxis( m_plotWidget->qwtPlot(),
                                                 RiaQDateTimeTools::supportedDateFormats().front(),
                                                 RiaQDateTimeTools::supportedTimeFormats().front(),
                                                 RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY,
                                                 RiaDefines::TimeFormatComponents::TIME_FORMAT_HOUR_MINUTE_SECOND );*/
    RiuQwtPlotTools::enableDateBasedBottomXAxis( m_plotWidget->qwtPlot(),
                                                 RiaQDateTimeTools::supportedDateFormats().front(),
                                                 QString(),
                                                 RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY,
                                                 RiaDefines::TimeFormatComponents::TIME_FORMAT_NONE );

    updateLegend();
    onLoadDataAndUpdate();

    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistoryWellAllocationPlot::deleteViewWidget()
{
    if ( m_plotWidget != nullptr )
    {
        m_plotWidget->hide(); // TODO: Hide or not hide?
        m_plotWidget->setParent( nullptr );
        delete m_plotWidget;
        m_plotWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistoryWellAllocationPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();

    if ( m_plotWidget == nullptr || m_case == nullptr )
    {
        return;
    }

    // If no 3D view is open, we have to make sure the case is opened
    if ( !m_case->ensureReservoirCaseIsOpen() )
    {
        return;
    }

    updateFromWell();
}

//--------------------------------------------------------------------------------------------------
/// TODO: Improve generating axis title, see WellAllocationPlot for how to retrieve units
//--------------------------------------------------------------------------------------------------
QString RimHistoryWellAllocationPlot::getYAxisTitleFromValueType() const
{
    if ( m_flowValueType == FlowValueType::PERCENTAGE )
    {
        return QString( "Percentage of well flow [%]" );
    }
    if ( m_flowValueType == FlowValueType::FLOW_RATE )
    {
        return QString( "Flow Rate [m<sup>3</sup>/day]" );
    }
    if ( m_flowValueType == FlowValueType::FLOW_VOLUME )
    {
        return QString( "Flow Volume [m<sup>3</sup>]" );
    }
    return QString( "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistoryWellAllocationPlot::updateFromWell()
{
    // TODO:
    // - Add branch detection/ branch count - see RimWellAllocationPlot
    if ( !m_plotWidget )
    {
        return;
    }
    m_plotWidget->insertLegend( RiuPlotWidget::Legend::BOTTOM );
    m_plotWidget->detachItems( RiuPlotWidget::PlotItemType::CURVE );

    // Retrieve collection of total fraction data for wells
    RimHistoryWellFlowDataCollection wellFlowDataCollection = createHistoryWellFlowDataCollection();
    std::vector<double>              allStackedValues( wellFlowDataCollection.timeStepDates().size(), 0.0 );

    // Negative z-position to show grid lines
    int zPos = -10000;
    for ( auto& [wellName, wellValues] : wellFlowDataCollection.wellValuesMap() )
    {
        cvf::Color3f color = m_flowDiagSolution ? m_flowDiagSolution->tracerColor( wellName ) : getTracerColor( wellName );
        for ( size_t i = 0; i < wellFlowDataCollection.timeStepDates().size(); ++i )
        {
            const auto value = wellValues.at( wellFlowDataCollection.timeStepDates()[i] );
            allStackedValues[i] += value;
        }

        const auto   qColor    = QColor( color.rByte(), color.gByte(), color.bByte() );
        const auto   fillColor = RiaColorTools::blendQColors( qColor, QColor( Qt::white ), 3, 1 );
        const QBrush fillBrush( fillColor, Qt::BrushStyle::SolidPattern );
        auto         interpolationType = m_flowValueType == FlowValueType::ACCUMULATED_FLOW_VOLUME
                                     ? RiuQwtPlotCurveDefines::CurveInterpolationEnum::INTERPOLATION_POINT_TO_POINT
                                     : RiuQwtPlotCurveDefines::CurveInterpolationEnum::INTERPOLATION_STEP_LEFT;

        RiuPlotCurve* curve = m_plotWidget->createPlotCurve( nullptr, wellName, qColor );
        curve->setAppearance( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID, interpolationType, 2, qColor, fillBrush );
        curve->setSamplesFromDatesAndYValues( wellFlowDataCollection.timeStepDates(), allStackedValues, false );
        curve->attachToPlot( m_plotWidget );
        curve->showInPlot();
        curve->setZ( zPos-- );
    }

    QString flowTypeText = m_flowDiagSolution() ? "Well Allocation History" : "Well Flow History";
    setDescription( flowTypeText + ": " + m_wellName + ", " + " (" + m_case->caseUserDescription() + ")" );
    m_plotWidget->setPlotTitle( m_userName );

    m_plotWidget->setAxisTitleText( RiuPlotAxis::defaultLeft(), getYAxisTitleFromValueType() );
    m_plotWidget->scheduleReplot();
}

//--------------------------------------------------------------------------------------------------
/// Create well flow calculator per time step date, retrieve total tracer fractions and propagate
/// well data for all time steps. If well does not exist for specific time step date - value is
/// set to 0.
//--------------------------------------------------------------------------------------------------
RimHistoryWellFlowDataCollection RimHistoryWellAllocationPlot::createHistoryWellFlowDataCollection()
{
    if ( !m_case )
    {
        return RimHistoryWellFlowDataCollection( {}, {} );
    }
    const RigSimWellData* simWellData = m_case->eclipseCaseData()->findSimWellData( m_wellName );
    if ( !simWellData )
    {
        return RimHistoryWellFlowDataCollection( {}, {} );
    }

    std::vector<QDateTime>                        timeSteps                  = m_case->timeStepDates();
    std::map<QDateTime, RigAccWellFlowCalculator> timeStepAndCalculatorPairs = {};

    // Note: Threshold per calculator does not work for accumulated data - use no threshold for each calculator
    // and filter on threshold value based on accumulated values at last time sample.
    const double smallContributionThreshold = m_groupSmallContributions() &&
                                                      m_flowValueType != FlowValueType::ACCUMULATED_FLOW_VOLUME
                                                  ? m_smallContributionsThreshold
                                                  : 0.0;
    for ( size_t i = 0; i < timeSteps.size(); ++i )
    {
        std::vector<std::vector<cvf::Vec3d>>          pipeBranchesCLCoords;
        std::vector<std::vector<RigWellResultPoint>>  pipeBranchesCellIds;
        std::map<QString, const std::vector<double>*> tracerFractionCellValues =
            RimWellAllocationTools::findOrCreateRelevantTracerCellFractions( simWellData, m_flowDiagSolution, i );

        RigSimulationWellCenterLineCalculator::calculateWellPipeCenterlineFromWellFrame( m_case->eclipseCaseData(),
                                                                                         simWellData,
                                                                                         i,
                                                                                         m_branchDetection,
                                                                                         true,
                                                                                         pipeBranchesCLCoords,
                                                                                         pipeBranchesCellIds );

        if ( tracerFractionCellValues.size() )
        {
            bool isProducer =
                ( simWellData->wellProductionType( i ) == RiaDefines::WellProductionType::PRODUCER ||
                  simWellData->wellProductionType( i ) == RiaDefines::WellProductionType::UNDEFINED_PRODUCTION_TYPE );
            RigEclCellIndexCalculator cellIdxCalc( m_case->eclipseCaseData()->mainGrid(),
                                                   m_case->eclipseCaseData()->activeCellInfo(
                                                       RiaDefines::PorosityModelType::MATRIX_MODEL ) );
            const auto                calculator = RigAccWellFlowCalculator( pipeBranchesCLCoords,

                                                              pipeBranchesCellIds,
                                                              tracerFractionCellValues,
                                                              cellIdxCalc,
                                                              smallContributionThreshold,
                                                              isProducer );
            timeStepAndCalculatorPairs.emplace( timeSteps[i], calculator );
        }
        else if ( pipeBranchesCLCoords.size() > 0 )
        {
            const auto calculator =
                RigAccWellFlowCalculator( pipeBranchesCLCoords, pipeBranchesCellIds, smallContributionThreshold );
            // NOTE: Would like to prevent this check. Is added due to calculator.tracerNames() gives
            // "oil", "water" and "gas" as return value when calculator.totalTracerFractions().size() = 0
            if ( calculator.totalTracerFractions().size() > 0 )
            {
                timeStepAndCalculatorPairs.emplace( timeSteps[i], calculator );
            }
        }
    }

    std::sort( timeSteps.begin(), timeSteps.end() );

    // Create collection
    RimHistoryWellFlowDataCollection wellFlowDataCollection( timeSteps, timeStepAndCalculatorPairs );

    if ( m_flowValueType == FlowValueType::PERCENTAGE )
    {
        wellFlowDataCollection.fillWithPercentageValues();
    }
    else if ( m_flowValueType == FlowValueType::FLOW_RATE )
    {
        wellFlowDataCollection.fillWithFlowRateValues();
    }
    else if ( m_flowValueType == FlowValueType::FLOW_VOLUME )
    {
        wellFlowDataCollection.fillWithFlowVolumeValues();
    }
    else if ( m_flowValueType == FlowValueType::ACCUMULATED_FLOW_VOLUME )
    {
        // Threshold for accumulated data is based on last time sample values after accumulating non-filtered flow volumes
        const double actualSmallContributionThreshold = m_groupSmallContributions() ? m_smallContributionsThreshold : 0.0;
        wellFlowDataCollection.fillWithAccumulatedFlowVolumeValues( actualSmallContributionThreshold );
    }
    else
    {
        CAF_ASSERT( "Not handled FlowValue type!" );
    }

    return wellFlowDataCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimHistoryWellAllocationPlot::userDescriptionField()
{
    return &m_userName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistoryWellAllocationPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimPlot::defineUiOrdering( uiConfigName, uiOrdering );

    uiOrdering.add( &m_userName );
    uiOrdering.add( &m_showPlotTitle );

    caf::PdmUiGroup& dataGroup = *uiOrdering.addNewGroup( "Plot Data" );
    dataGroup.add( &m_case );
    dataGroup.add( &m_wellName );
    dataGroup.add( &m_branchDetection );

    caf::PdmUiGroup& optionGroup = *uiOrdering.addNewGroup( "Options" );
    optionGroup.add( &m_flowDiagSolution );
    optionGroup.add( &m_flowValueType );
    optionGroup.add( &m_groupSmallContributions );
    optionGroup.add( &m_smallContributionsThreshold );
    m_smallContributionsThreshold.uiCapability()->setUiReadOnly( !m_groupSmallContributions() );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimHistoryWellAllocationPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                     const QVariant&            oldValue,
                                                     const QVariant&            newValue )
{
    RimPlot::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_case )
    {
        if ( m_flowDiagSolution && m_case )
        {
            m_flowDiagSolution = m_case->defaultFlowDiagSolution();
        }
        else
        {
            m_flowDiagSolution = nullptr;
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
    else if ( changedField == &m_wellName || changedField == &m_flowDiagSolution || changedField == &m_flowValueType ||
              changedField == &m_branchDetection || changedField == &m_groupSmallContributions ||
              changedField == &m_smallContributionsThreshold )
    {
        onLoadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RimHistoryWellAllocationPlot::findSortedWellNames()
{
    if ( m_case && m_case->eclipseCaseData() )
    {
        return m_case->eclipseCaseData()->findSortedWellNames();
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimHistoryWellAllocationPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options = RimPlot::calculateValueOptions( fieldNeedingOptions );

    if ( fieldNeedingOptions == &m_case )
    {
        auto resultCases = RimEclipseCaseTools::eclipseResultCases();
        for ( RimEclipseResultCase* c : resultCases )
        {
            options.push_back( caf::PdmOptionItemInfo( c->caseUserDescription(), c, false, c->uiIconProvider() ) );
        }
    }
    else if ( fieldNeedingOptions == &m_wellName )
    {
        const std::set<QString> sortedWellNames = findSortedWellNames();
        caf::IconProvider       simWellIcon( ":/Well.svg" );
        for ( const auto& name : sortedWellNames )
        {
            options.push_back( caf::PdmOptionItemInfo( name, name, false, simWellIcon ) );
        }
        if ( options.size() == 0 )
        {
            options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
        }
    }
    else if ( fieldNeedingOptions == &m_flowDiagSolution && m_case )
    {
        RimFlowDiagSolution* defaultFlowSolution = m_case->defaultFlowDiagSolution();
        options.push_back( caf::PdmOptionItemInfo( "Well Flow", nullptr ) );
        if ( defaultFlowSolution )
        {
            options.push_back( caf::PdmOptionItemInfo( "Allocation", defaultFlowSolution ) );
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimHistoryWellAllocationPlot::getTracerColor( const QString& tracerName )
{
    if ( tracerName == RIG_FLOW_OIL_NAME ) return cvf::Color3f::DARK_GREEN;
    if ( tracerName == RIG_FLOW_GAS_NAME ) return cvf::Color3f::DARK_RED;
    if ( tracerName == RIG_FLOW_WATER_NAME ) return cvf::Color3f::BLUE;
    return cvf::Color3f::DARK_GRAY;
}
