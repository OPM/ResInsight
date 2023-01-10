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

#include "RimWellAllocationOverTimePlot.h"

#include "RiaColorTools.h"
#include "RiaDefines.h"
#include "RiaPreferences.h"
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
#include "RimSimWellInView.h"
#include "RimStackablePlotCurve.h"
#include "RimWellAllocationOverTimeCollection.h"
#include "RimWellAllocationTools.h"
#include "RimWellLogFile.h"
#include "RimWellPlotTools.h"

#include "RiuContextMenuLauncher.h"
#include "RiuPlotCurve.h"
#include "RiuPlotWidget.h"
#include "RiuQwtCurvePointTracker.h"
#include "RiuQwtPlotTools.h"
#include "RiuQwtPlotWidget.h"

#include "cafCmdFeatureMenuBuilder.h"

CAF_PDM_SOURCE_INIT( RimWellAllocationOverTimePlot, "RimWellAllocationOverTimePlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
namespace caf
{
template <>
void AppEnum<RimWellAllocationOverTimePlot::FlowValueType>::setUp()
{
    addItem( RimWellAllocationOverTimePlot::FLOW_RATE, "FLOW_RATE", "Flow Rates" );
    addItem( RimWellAllocationOverTimePlot::FLOW_RATE_PERCENTAGE, "FLOW_RATE_PERCENTAGE", "Flow Rate Percentage" );
    addItem( RimWellAllocationOverTimePlot::FLOW_VOLUME, "FLOW_VOLUME", "Flow Volumes" );
    addItem( RimWellAllocationOverTimePlot::ACCUMULATED_FLOW_VOLUME, "ACCUMULATED_FLOW_VOLUME", "Accumulated Flow Volumes" );
    addItem( RimWellAllocationOverTimePlot::ACCUMULATED_FLOW_VOLUME_PERCENTAGE,
             "ACCUMULATED_FLOW_VOLUME_PERCENTAGE",
             "Accumulated Flow Volume Percentage" );
    setDefault( RimWellAllocationOverTimePlot::FLOW_RATE );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellAllocationOverTimePlot::RimWellAllocationOverTimePlot()
{
    // TODO: Add icon
    CAF_PDM_InitObject( "Well Allocation Over Time Plot", ":/WellAllocOverTimePlot16x16.png" );

    CAF_PDM_InitField( &m_userName, "PlotDescription", QString( "Well Allocation Over Time Plot" ), "Name" );
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
RimWellAllocationOverTimePlot::~RimWellAllocationOverTimePlot()
{
    removeMdiWindowFromMdiArea();
    deleteViewWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellAllocationOverTimePlot::setDescription( const QString& description )
{
    m_userName = description;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellAllocationOverTimePlot::setFromSimulationWell( RimSimWellInView* simWell )
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
RiuPlotWidget* RimWellAllocationOverTimePlot::plotWidget()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellAllocationOverTimePlot::asciiDataForPlotExport() const
{
    // Retrieve collection of allocation over time data for wells
    RimWellAllocationOverTimeCollection allocationOverTimeCollection = createWellAllocationOverTimeCollection();

    QString titleText = m_userName + "\n\n";

    QString dataText = "Time Step\t";
    for ( auto& [wellName, wellValues] : allocationOverTimeCollection.wellValuesMap() )
    {
        dataText += wellName + "\t";
    }
    dataText += "\n";

    const QString dateFormatStr =
        RiaQDateTimeTools::dateFormatString( RiaPreferences::current()->dateFormat(),
                                             RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY );
    for ( const auto& timeStep : allocationOverTimeCollection.timeStepDates() )
    {
        dataText += timeStep.toString( dateFormatStr ) + "\t";
        for ( auto& [wellName, wellValues] : allocationOverTimeCollection.wellValuesMap() )
        {
            dataText += wellValues.count( timeStep ) == 0 ? QString::number( 0.0 )
                                                          : QString::number( wellValues.at( timeStep ) );
            dataText += "\t";
        }
        dataText += "\n";
    }

    return titleText + dataText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellAllocationOverTimePlot::description() const
{
    return uiName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimWellAllocationOverTimePlot::viewWidget()
{
    return plotWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimWellAllocationOverTimePlot::snapshotWindowContent()
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
RiuPlotWidget* RimWellAllocationOverTimePlot::doCreatePlotViewWidget( QWidget* mainWindowParent )
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

    caf::CmdFeatureMenuBuilder menuBuilder;
    menuBuilder << "RicShowPlotDataFeature";
    new RiuContextMenuLauncher( plotWidget, menuBuilder );

    m_plotWidget = plotWidget;
    m_plotWidget->setAxisTitleEnabled( RiuPlotAxis::defaultLeft(), true );

    RiuQwtPlotTools::enableDateBasedBottomXAxis( m_plotWidget->qwtPlot(),
                                                 RiaPreferences::current()->dateFormat(),
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
void RimWellAllocationOverTimePlot::deleteViewWidget()
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
void RimWellAllocationOverTimePlot::onLoadDataAndUpdate()
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
///
//--------------------------------------------------------------------------------------------------
QString RimWellAllocationOverTimePlot::getValueTypeText() const
{
    RiaDefines::EclipseUnitSystem     unitSet   = m_case->eclipseCaseData()->unitsType();
    RimWellLogFile::WellFlowCondition condition = m_flowDiagSolution ? RimWellLogFile::WELL_FLOW_COND_RESERVOIR
                                                                     : RimWellLogFile::WELL_FLOW_COND_STANDARD;

    if ( m_flowValueType == FlowValueType::FLOW_RATE_PERCENTAGE )
    {
        QString conditionText = condition == RimWellLogFile::WELL_FLOW_COND_RESERVOIR ? "Reservoir" : "Surface";
        return QString( "Percentage of %1 Flow Rate [%]" ).arg( conditionText );
    }
    if ( m_flowValueType == FlowValueType::FLOW_RATE )
    {
        return RimWellPlotTools::flowPlotAxisTitle( condition, unitSet );
    }
    if ( m_flowValueType == FlowValueType::FLOW_VOLUME )
    {
        return RimWellPlotTools::flowVolumePlotAxisTitle( condition, unitSet );
    }
    if ( m_flowValueType == FlowValueType::ACCUMULATED_FLOW_VOLUME )
    {
        return "Accumulated " + RimWellPlotTools::flowVolumePlotAxisTitle( condition, unitSet );
    }
    if ( m_flowValueType == FlowValueType::ACCUMULATED_FLOW_VOLUME_PERCENTAGE )
    {
        QString conditionText = condition == RimWellLogFile::WELL_FLOW_COND_RESERVOIR ? "Reservoir" : "Surface";
        return QString( "Accumulated %1 Flow Volume Allocation [%]" ).arg( conditionText );
    }

    return QString( "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellAllocationOverTimePlot::updateFromWell()
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
    RimWellAllocationOverTimeCollection allocationOverTimeCollection = createWellAllocationOverTimeCollection();
    std::vector<double>                 allStackedValues( allocationOverTimeCollection.timeStepDates().size(), 0.0 );

    // Negative z-position to show grid lines
    int zPos = -10000;
    for ( auto& [wellName, wellValues] : allocationOverTimeCollection.wellValuesMap() )
    {
        cvf::Color3f color = m_flowDiagSolution ? m_flowDiagSolution->tracerColor( wellName ) : getTracerColor( wellName );
        for ( size_t i = 0; i < allocationOverTimeCollection.timeStepDates().size(); ++i )
        {
            const auto value = wellValues.at( allocationOverTimeCollection.timeStepDates()[i] );
            allStackedValues[i] += value;
        }

        const auto   qColor    = QColor( color.rByte(), color.gByte(), color.bByte() );
        const auto   fillColor = RiaColorTools::blendQColors( qColor, QColor( Qt::white ), 3, 1 );
        const QBrush fillBrush( fillColor, Qt::BrushStyle::SolidPattern );
        auto         interpolationType = m_flowValueType == FlowValueType::ACCUMULATED_FLOW_VOLUME
                                     ? RiuQwtPlotCurveDefines::CurveInterpolationEnum::INTERPOLATION_POINT_TO_POINT
                                     : RiuQwtPlotCurveDefines::CurveInterpolationEnum::INTERPOLATION_STEP_LEFT;

        RiuPlotCurve* curve = m_plotWidget->createPlotCurve( nullptr, wellName );
        curve->setAppearance( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID, interpolationType, 2, qColor, fillBrush );
        curve->setSamplesFromDatesAndYValues( allocationOverTimeCollection.timeStepDates(), allStackedValues, false );
        curve->attachToPlot( m_plotWidget );
        curve->showInPlot();
        curve->setZ( zPos-- );
    }

    QString descriptionText = QString( m_flowDiagSolution() ? "Well Allocation Over Time: " : "Well Flow Over Time: " ) +
                              QString( "%1 (%2)" ).arg( m_wellName ).arg( m_case->caseUserDescription() );
    QString valueTypeText  = getValueTypeText();
    QString newDescription = descriptionText + ", " + valueTypeText;

    setDescription( newDescription );
    m_plotWidget->setWindowTitle( newDescription );
    m_plotWidget->setPlotTitle( descriptionText + "<br>" + valueTypeText + "</br>" );

    m_plotWidget->setAxisTitleText( RiuPlotAxis::defaultLeft(), valueTypeText );
    m_plotWidget->scheduleReplot();
}

//--------------------------------------------------------------------------------------------------
/// Create well flow calculator per time step date, retrieve total tracer fractions and propagate
/// well data for all time steps. If well does not exist for specific time step date - value is
/// set to 0.
//--------------------------------------------------------------------------------------------------
RimWellAllocationOverTimeCollection RimWellAllocationOverTimePlot::createWellAllocationOverTimeCollection() const
{
    if ( !m_case )
    {
        return RimWellAllocationOverTimeCollection( {}, {} );
    }
    const RigSimWellData* simWellData = m_case->eclipseCaseData()->findSimWellData( m_wellName );
    if ( !simWellData )
    {
        return RimWellAllocationOverTimeCollection( {}, {} );
    }

    std::vector<QDateTime>                        timeSteps                  = m_case->timeStepDates();
    std::map<QDateTime, RigAccWellFlowCalculator> timeStepAndCalculatorPairs = {};

    // Note: Threshold per calculator does not work for accumulated data - use no threshold for each calculator
    // and filter on threshold value based on accumulated values at last time sample.
    const bool neglectThresholdToAccumulateVolume = m_flowValueType == FlowValueType::ACCUMULATED_FLOW_VOLUME ||
                                                    m_flowValueType == FlowValueType::ACCUMULATED_FLOW_VOLUME_PERCENTAGE;
    const double smallContributionThreshold =
        m_groupSmallContributions() && !neglectThresholdToAccumulateVolume ? m_smallContributionsThreshold : 0.0;
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
    RimWellAllocationOverTimeCollection collection( timeSteps, timeStepAndCalculatorPairs );

    if ( m_flowValueType == FlowValueType::FLOW_RATE_PERCENTAGE )
    {
        collection.fillWithFlowRatePercentageValues();
    }
    else if ( m_flowValueType == FlowValueType::FLOW_RATE )
    {
        collection.fillWithFlowRateValues();
    }
    else if ( m_flowValueType == FlowValueType::FLOW_VOLUME )
    {
        collection.fillWithFlowVolumeValues();
    }
    else if ( m_flowValueType == FlowValueType::ACCUMULATED_FLOW_VOLUME )
    {
        // Accumulated flow volume without threshold, and filter according to threshold after accumulating volumes
        const double actualSmallContributionThreshold = m_groupSmallContributions() ? m_smallContributionsThreshold : 0.0;
        collection.fillWithAccumulatedFlowVolumeValues( actualSmallContributionThreshold );
    }
    else if ( m_flowValueType == FlowValueType::ACCUMULATED_FLOW_VOLUME_PERCENTAGE )
    {
        // Accumulate flow volume percentages without threshold, and filter according to threshold after accumulating
        // values
        const double actualSmallContributionThreshold = m_groupSmallContributions() ? m_smallContributionsThreshold : 0.0;
        collection.fillWithAccumulatedFlowVolumePercentageValues( actualSmallContributionThreshold );
    }
    else
    {
        CAF_ASSERT( "Not handled FlowValue type!" );
    }

    return collection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellAllocationOverTimePlot::userDescriptionField()
{
    return &m_userName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellAllocationOverTimePlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
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
void RimWellAllocationOverTimePlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
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
std::set<QString> RimWellAllocationOverTimePlot::findSortedWellNames()
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
    RimWellAllocationOverTimePlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
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
cvf::Color3f RimWellAllocationOverTimePlot::getTracerColor( const QString& tracerName )
{
    if ( tracerName == RIG_FLOW_OIL_NAME ) return cvf::Color3f::DARK_GREEN;
    if ( tracerName == RIG_FLOW_GAS_NAME ) return cvf::Color3f::DARK_RED;
    if ( tracerName == RIG_FLOW_WATER_NAME ) return cvf::Color3f::BLUE;
    return cvf::Color3f::DARK_GRAY;
}
