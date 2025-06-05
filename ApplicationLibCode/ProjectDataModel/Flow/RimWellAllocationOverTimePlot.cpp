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
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaQDateTimeTools.h"
#include "RiaStdStringTools.h"

#include "RigEclipseCaseData.h"
#include "Well/RigAccWellFlowCalculator.h"
#include "Well/RigSimWellData.h"
#include "Well/RigSimulationWellCenterLineCalculator.h"
#include "Well/RigWellAllocationOverTime.h"
#include "Well/RigWellResultPoint.h"

#include "RimEclipseCaseTools.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimFlowDiagSolution.h"
#include "RimSimWellInView.h"
#include "RimStackablePlotCurve.h"
#include "RimWellAllocationTools.h"
#include "RimWellLogLasFile.h"
#include "RimWellPlotTools.h"

#include "RiuContextMenuLauncher.h"
#include "RiuPlotCurve.h"
#include "RiuPlotWidget.h"
#include "RiuQwtCurvePointTracker.h"
#include "RiuQwtPlotTools.h"
#include "RiuQwtPlotWidget.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafFontTools.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include "qwt_scale_draw.h"

CAF_PDM_SOURCE_INIT( RimWellAllocationOverTimePlot, "RimWellAllocationOverTimePlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
namespace caf
{
template <>
void AppEnum<RimWellAllocationOverTimePlot::FlowValueType>::setUp()
{
    addItem( RimWellAllocationOverTimePlot::FlowValueType::FLOW_RATE, "FLOW_RATE", "Flow Rates" );
    addItem( RimWellAllocationOverTimePlot::FlowValueType::FLOW_RATE_PERCENTAGE, "FLOW_RATE_PERCENTAGE", "Flow Rate Percentage" );
    addItem( RimWellAllocationOverTimePlot::FlowValueType::FLOW_VOLUME, "FLOW_VOLUME", "Flow Volumes" );
    addItem( RimWellAllocationOverTimePlot::FlowValueType::ACCUMULATED_FLOW_VOLUME, "ACCUMULATED_FLOW_VOLUME", "Accumulated Flow Volumes" );
    addItem( RimWellAllocationOverTimePlot::FlowValueType::ACCUMULATED_FLOW_VOLUME_PERCENTAGE,
             "ACCUMULATED_FLOW_VOLUME_PERCENTAGE",
             "Accumulated Flow Volume Percentage" );
    setDefault( RimWellAllocationOverTimePlot::FlowValueType::FLOW_RATE );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
namespace caf
{
template <>
void AppEnum<RimWellAllocationOverTimePlot::TimeStepFilterMode>::setUp()
{
    addItem( RimWellAllocationOverTimePlot::TimeStepFilterMode::NONE, "NONE", "Show All Time Steps" );
    addItem( RimWellAllocationOverTimePlot::TimeStepFilterMode::TIME_STEP_COUNT, "TIME_STEP_COUNT", "Time Step Count" );
    setDefault( RimWellAllocationOverTimePlot::TimeStepFilterMode::TIME_STEP_COUNT );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellAllocationOverTimePlot::RimWellAllocationOverTimePlot()
{
    CAF_PDM_InitObject( "Well Allocation Over Time Plot", ":/AnalysisPlot16x16.png" );

    CAF_PDM_InitField( &m_userName, "PlotDescription", QString( "Well Allocation Over Time Plot" ), "Name" );
    m_userName.uiCapability()->setUiReadOnly( true );
    CAF_PDM_InitFieldNoDefault( &m_case, "CurveCase", "Case" );
    m_case.uiCapability()->setUiTreeChildrenHidden( true );
    CAF_PDM_InitField( &m_wellName, "WellName", QString( "None" ), "Well" );

    CAF_PDM_InitFieldNoDefault( &m_selectedFromTimeStep, "FromTimeStep", "From Time Step" );
    m_selectedFromTimeStep.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    CAF_PDM_InitFieldNoDefault( &m_selectedToTimeStep, "ToTimeStep", "To Time Step" );
    m_selectedToTimeStep.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    CAF_PDM_InitFieldNoDefault( &m_timeStepFilterMode, "TimeStepRangeFilterMode", "Filter" );
    m_timeStepFilterMode.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_timeStepCount, "TimeStepCount", m_initialNumberOfTimeSteps, "Number of Time Steps" );
    CAF_PDM_InitFieldNoDefault( &m_excludeTimeSteps, "ExcludeTimeSteps", "" );
    m_excludeTimeSteps.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    CAF_PDM_InitFieldNoDefault( &m_applyTimeStepSelections, "ApplyTimeStepSelections", "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelLeft( &m_applyTimeStepSelections );

    CAF_PDM_InitFieldNoDefault( &m_flowDiagSolution, "FlowDiagSolution", "Plot Type" );
    CAF_PDM_InitFieldNoDefault( &m_flowValueType, "FlowValueType", "Value Type" );
    CAF_PDM_InitField( &m_groupSmallContributions, "GroupSmallContributions", true, "Group Small Contributions" );
    CAF_PDM_InitField( &m_smallContributionsThreshold, "SmallContributionsThreshold", 0.005, "Threshold" );

    CAF_PDM_InitFieldNoDefault( &m_axisTitleFontSize, "AxisTitleFontSize", "Axis Title Font Size" );
    CAF_PDM_InitFieldNoDefault( &m_axisValueFontSize, "AxisValueFontSize", "Axis Value Font Size" );
    m_axisTitleFontSize = caf::FontTools::RelativeSize::Large;
    m_axisValueFontSize = caf::FontTools::RelativeSize::Medium;
    m_legendFontSize    = caf::FontTools::RelativeSize::Medium;

    setAsPlotMdiWindow();
    setShowWindow( false );
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
    auto eclView = simWell->firstAncestorOrThisOfType<RimEclipseView>();
    auto eclCase = simWell->firstAncestorOrThisOfType<RimEclipseResultCase>();

    m_case     = eclCase;
    m_wellName = simWell->simWellData()->m_wellName;

    setValidTimeStepRangeForCase();

    // Use the active flow diagnostics solutions, or the first one as default
    m_flowDiagSolution = eclView->cellResult()->flowDiagSolution();
    if ( ( m_flowDiagSolution == nullptr ) && ( m_case != nullptr ) )
    {
        m_flowDiagSolution = m_case->defaultFlowDiagSolution();
    }

    onLoadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellAllocationOverTimePlot::setWellName( const QString& wellName )
{
    m_wellName = wellName;
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
    // Retrieve allocation over time data for wells
    RigWellAllocationOverTime allocationOverTime = createWellAllocationOverTime();

    QString titleText = m_userName + "\n\n";

    QString dataText = "Time Step\t";
    for ( auto& [wellName, wellValues] : allocationOverTime.wellValuesMap() )
    {
        dataText += wellName + "\t";
    }
    dataText += "\n";

    const QString dateFormatStr = dateFormatString();
    for ( const auto& timeStep : allocationOverTime.timeStepDates() )
    {
        dataText += timeStep.toString( dateFormatStr ) + "\t";
        for ( auto& [wellName, wellValues] : allocationOverTime.wellValuesMap() )
        {
            dataText += wellValues.count( timeStep ) == 0 ? QString::number( 0.0 ) : QString::number( wellValues.at( timeStep ) );
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

    new RiuContextMenuLauncher( plotWidget, { "RicShowPlotDataFeature" } );

    m_plotWidget = plotWidget;
    RiuQwtPlotTools::enableDateBasedBottomXAxis( m_plotWidget->qwtPlot(),
                                                 RiaPreferences::current()->dateFormat(),
                                                 QString(),
                                                 RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY,
                                                 RiaDefines::TimeFormatComponents::TIME_FORMAT_NONE );

    // Workaround: Enable axis title for bottom axis to activate correct font size for date axis
    m_plotWidget->setAxisTitleEnabled( RiuPlotAxis::defaultBottom(), true );
    m_plotWidget->setAxisTitleEnabled( RiuPlotAxis::defaultLeft(), true );

    if ( auto qwtPlot = m_plotWidget->qwtPlot() )
    {
        auto scaleDraw = qwtPlot->axisScaleDraw( QwtAxis::XBottom );
        if ( scaleDraw )
        {
            scaleDraw->setLabelRotation( 30.0 );
        }
    }

    m_plotWidget->insertLegend( RiuPlotWidget::Legend::RIGHT );

    m_plotWidget->setAxisTitleText( RiuPlotAxis::defaultBottom(), "[Date]" );

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
        m_plotWidget->hide();
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

    // Must be called after curves are added to plot - to apply correct legend font size
    doUpdateLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellAllocationOverTimePlot::doUpdateLayout()
{
    if ( m_plotWidget == nullptr )
    {
        return;
    }

    m_plotWidget->setPlotTitleFontSize( titleFontSize() );
    m_plotWidget->setLegendFontSize( legendFontSize() );
    m_plotWidget->setAxisFontsAndAlignment( RiuPlotAxis::defaultBottom(), axisTitleFontSize(), axisValueFontSize() );
    m_plotWidget->setAxisFontsAndAlignment( RiuPlotAxis::defaultLeft(), axisTitleFontSize(), axisValueFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellAllocationOverTimePlot::updateFromWell()
{
    if ( !m_plotWidget )
    {
        return;
    }
    m_plotWidget->detachItems( RiuPlotWidget::PlotItemType::CURVE );

    // Retrieve total fraction data for wells
    RigWellAllocationOverTime allocationOverTime = createWellAllocationOverTime();
    std::vector<double>       allStackedValues( allocationOverTime.timeStepDates().size(), 0.0 );

    // Negative z-position to show grid lines
    int zPos = -10000;
    for ( auto& [wellName, wellValues] : allocationOverTime.wellValuesMap() )
    {
        cvf::Color3f color = m_flowDiagSolution ? m_flowDiagSolution->tracerColor( wellName ) : getTracerColor( wellName );
        for ( size_t i = 0; i < allocationOverTime.timeStepDates().size(); ++i )
        {
            const auto value = wellValues.at( allocationOverTime.timeStepDates()[i] );
            allStackedValues[i] += value;
        }

        const auto   qColor    = QColor( color.rByte(), color.gByte(), color.bByte() );
        const auto   fillColor = RiaColorTools::blendQColors( qColor, QColor( Qt::white ), 3, 1 );
        const QBrush fillBrush( fillColor, Qt::BrushStyle::SolidPattern );
        auto         interpolationType = m_flowValueType == FlowValueType::ACCUMULATED_FLOW_VOLUME ||
                                         m_flowValueType == FlowValueType::ACCUMULATED_FLOW_VOLUME_PERCENTAGE
                                             ? RiuQwtPlotCurveDefines::CurveInterpolationEnum::INTERPOLATION_POINT_TO_POINT
                                             : RiuQwtPlotCurveDefines::CurveInterpolationEnum::INTERPOLATION_STEP_LEFT;

        RiuPlotCurve* curve = m_plotWidget->createPlotCurve( nullptr, wellName );
        curve->setAppearance( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID, interpolationType, 2, qColor, fillBrush );
        curve->setSamplesFromDatesAndYValues( allocationOverTime.timeStepDates(), allStackedValues, false );
        curve->attachToPlot( m_plotWidget );
        curve->showInPlot();
        curve->setZ( zPos-- );
    }

    QString descriptionText = QString( m_flowDiagSolution() ? "Well Allocation Over Time: " : "Well Flow Over Time: " ) +
                              QString( "%1 (%2)" ).arg( m_wellName ).arg( m_case->caseUserDescription() );
    QString valueTypeText  = getValueTypeText();
    QString newDescription = descriptionText + ", " + valueTypeText;

    setDescription( newDescription );

    const auto windowTitle = RiaStdStringTools::removeHtmlTags( newDescription.toStdString() );
    m_plotWidget->setWindowTitle( QString::fromStdString( windowTitle ) );

    m_plotWidget->setPlotTitle( descriptionText + "<br>" + valueTypeText + "</br>" );
    m_plotWidget->setAxisTitleText( RiuPlotAxis::defaultLeft(), valueTypeText );

    if ( m_plotWidget->qwtPlot() )
    {
        m_plotWidget->qwtPlot()->replot();

        // Workaround: For some reason, the ordering of items is not updated correctly. Adjusting the z-value of curves
        // will trigger a replot of items in correct order.

        auto plotItemList = m_plotWidget->qwtPlot()->itemList();
        for ( QwtPlotItem* plotItem : plotItemList )
        {
            auto zValue = plotItem->z() + 1;
            plotItem->setZ( zValue );
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Create well flow calculator per time step date, retrieve total tracer fractions and propagate
/// well data for all time steps. If well does not exist for specific time step date - value is
/// set to 0.
//--------------------------------------------------------------------------------------------------
RigWellAllocationOverTime RimWellAllocationOverTimePlot::createWellAllocationOverTime() const
{
    if ( !m_case )
    {
        return RigWellAllocationOverTime( {}, {} );
    }
    if ( m_selectedFromTimeStep() > m_selectedToTimeStep() )
    {
        RiaLogging::error( QString( "Selected 'From Time Step' (%1) must be prior to selected 'To Time Step' (%2)" )
                               .arg( m_selectedFromTimeStep().toString( dateFormatString() ) )
                               .arg( m_selectedToTimeStep().toString( dateFormatString() ) ) );
        return RigWellAllocationOverTime( {}, {} );
    }
    const RigSimWellData* simWellData = m_case->eclipseCaseData()->findSimWellData( m_wellName );
    if ( !simWellData )
    {
        return RigWellAllocationOverTime( {}, {} );
    }

    // Note: Threshold per calculator does not work for accumulated data - use no threshold for each calculator
    // and filter on threshold value after accumulating non-filtered values.
    const double smallContributionThreshold = m_groupSmallContributions() && m_flowValueType != FlowValueType::ACCUMULATED_FLOW_VOLUME &&
                                                      m_flowValueType != FlowValueType::ACCUMULATED_FLOW_VOLUME_PERCENTAGE
                                                  ? m_smallContributionsThreshold
                                                  : 0.0;

    const bool                                    branchDetection = false;
    std::map<QDateTime, RigAccWellFlowCalculator> timeStepAndCalculatorPairs;
    std::vector<QDateTime>                        allTimeSteps      = m_case->timeStepDates();
    std::set<QDateTime>                           selectedTimeSteps = getSelectedTimeSteps( allTimeSteps );
    std::set<QDateTime>                           excludedTimeSteps = std::set( m_excludeTimeSteps().begin(), m_excludeTimeSteps().end() );

    // NOTE: Must iterate all time step dates for case to have correct time step index for simulation well data
    for ( size_t i = 0; i < allTimeSteps.size(); ++i )
    {
        // Time step must be among selected time steps and not excluded
        if ( selectedTimeSteps.find( allTimeSteps[i] ) == selectedTimeSteps.end() ||
             excludedTimeSteps.find( allTimeSteps[i] ) != excludedTimeSteps.end() )
        {
            continue;
        }

        std::map<QString, const std::vector<double>*> tracerFractionCellValues =
            RimWellAllocationTools::findOrCreateRelevantTracerCellFractions( simWellData, m_flowDiagSolution, static_cast<int>( i ) );

        auto simWellBranches = RigSimulationWellCenterLineCalculator::calculateWellPipeCenterlineForTimeStep( m_case->eclipseCaseData(),
                                                                                                              simWellData,
                                                                                                              static_cast<int>( i ),
                                                                                                              branchDetection,
                                                                                                              true );

        const auto& [pipeBranchesCLCoords, pipeBranchesCellIds] = RigSimulationWellCenterLineCalculator::extractBranchData( simWellBranches );

        if ( !tracerFractionCellValues.empty() && !pipeBranchesCLCoords.empty() )
        {
            bool                      isProducer = ( simWellData->wellProductionType( i ) == RiaDefines::WellProductionType::PRODUCER ||
                                simWellData->wellProductionType( i ) == RiaDefines::WellProductionType::UNDEFINED_PRODUCTION_TYPE );
            RigEclCellIndexCalculator cellIdxCalc( m_case->eclipseCaseData()->mainGrid(),
                                                   m_case->eclipseCaseData()->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL ),
                                                   nullptr );
            const auto                calculator = RigAccWellFlowCalculator( pipeBranchesCLCoords,
                                                              pipeBranchesCellIds,
                                                              tracerFractionCellValues,
                                                              cellIdxCalc,
                                                              smallContributionThreshold,
                                                              isProducer );
            timeStepAndCalculatorPairs.emplace( allTimeSteps[i], calculator );
        }
        else if ( !pipeBranchesCLCoords.empty() )
        {
            const auto calculator = RigAccWellFlowCalculator( pipeBranchesCLCoords, pipeBranchesCellIds, smallContributionThreshold );
            // NOTE: Would like to prevent this check. Is added due to calculator.tracerNames() gives
            // "oil", "water" and "gas" as return value when calculator.totalTracerFractions().size() = 0
            if ( !calculator.totalTracerFractions().empty() )
            {
                timeStepAndCalculatorPairs.emplace( allTimeSteps[i], calculator );
            }
        }
    }

    // Create well allocation over time data
    const auto                selectedTimeStepsVector = std::vector( selectedTimeSteps.begin(), selectedTimeSteps.end() );
    RigWellAllocationOverTime wellAllocationOverTime( selectedTimeStepsVector, timeStepAndCalculatorPairs );

    if ( m_flowValueType == FlowValueType::FLOW_RATE_PERCENTAGE )
    {
        wellAllocationOverTime.fillWithFlowRatePercentageValues();
    }
    else if ( m_flowValueType == FlowValueType::FLOW_RATE )
    {
        wellAllocationOverTime.fillWithFlowRateValues();
    }
    else if ( m_flowValueType == FlowValueType::FLOW_VOLUME )
    {
        wellAllocationOverTime.fillWithFlowVolumeValues();
    }
    else if ( m_flowValueType == FlowValueType::ACCUMULATED_FLOW_VOLUME )
    {
        // Accumulated flow volume without threshold, and filter according to threshold after accumulating volumes
        const double actualSmallContributionThreshold = m_groupSmallContributions() ? m_smallContributionsThreshold : 0.0;
        wellAllocationOverTime.fillWithAccumulatedFlowVolumeValues( actualSmallContributionThreshold );
    }
    else if ( m_flowValueType == FlowValueType::ACCUMULATED_FLOW_VOLUME_PERCENTAGE )
    {
        // Accumulate flow volume percentages without threshold, and filter according to threshold after accumulating
        // values
        const double actualSmallContributionThreshold = m_groupSmallContributions() ? m_smallContributionsThreshold : 0.0;
        wellAllocationOverTime.fillWithAccumulatedFlowVolumePercentageValues( actualSmallContributionThreshold );
    }
    else
    {
        CAF_ASSERT( "Not handled FlowValue type!" );
    }

    return wellAllocationOverTime;
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

    caf::PdmUiGroup& timeStepGroup = *uiOrdering.addNewGroup( "Time Step" );
    timeStepGroup.add( &m_selectedFromTimeStep );
    timeStepGroup.add( &m_selectedToTimeStep );
    timeStepGroup.add( &m_timeStepFilterMode );

    if ( m_timeStepFilterMode == TimeStepFilterMode::TIME_STEP_COUNT )
    {
        timeStepGroup.add( &m_timeStepCount );
    }

    caf::PdmUiGroup& excludeTimeStepGroup = *timeStepGroup.addNewGroup( "Exclude Time Steps" );
    excludeTimeStepGroup.add( &m_excludeTimeSteps );
    excludeTimeStepGroup.setCollapsedByDefault();

    timeStepGroup.add( &m_applyTimeStepSelections );

    caf::PdmUiGroup& optionGroup = *uiOrdering.addNewGroup( "Options" );
    optionGroup.add( &m_flowDiagSolution );
    optionGroup.add( &m_flowValueType );
    optionGroup.add( &m_groupSmallContributions );
    optionGroup.add( &m_smallContributionsThreshold );
    m_smallContributionsThreshold.uiCapability()->setUiReadOnly( !m_groupSmallContributions() );

    caf::PdmUiGroup* fontGroup = uiOrdering.addNewGroup( "Fonts" );
    fontGroup->setCollapsedByDefault();
    RimPlotWindow::uiOrderingForFonts( uiConfigName, *fontGroup );
    fontGroup->add( &m_axisTitleFontSize );
    fontGroup->add( &m_axisValueFontSize );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellAllocationOverTimePlot::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                           QString                    uiConfigName,
                                                           caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_applyTimeStepSelections )
    {
        caf::PdmUiPushButtonEditorAttribute* attrib = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->m_buttonText = "Apply";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellAllocationOverTimePlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
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
        if ( sortedWellNames.empty() )
            m_wellName = "";
        else if ( sortedWellNames.count( m_wellName() ) == 0 )
        {
            m_wellName = *sortedWellNames.begin();
        }

        setValidTimeStepRangeForCase();
        onLoadDataAndUpdate();
    }
    else if ( changedField == &m_wellName || changedField == &m_flowDiagSolution || changedField == &m_flowValueType ||
              changedField == &m_groupSmallContributions || changedField == &m_smallContributionsThreshold )
    {
        onLoadDataAndUpdate();
    }
    else if ( changedField == &m_timeStepCount && m_timeStepFilterMode == TimeStepFilterMode::TIME_STEP_COUNT )
    {
        m_excludeTimeSteps.setValue( {} );
    }
    else if ( changedField == &m_applyTimeStepSelections )
    {
        onLoadDataAndUpdate();
    }
    else if ( changedField == &m_axisTitleFontSize || changedField == &m_axisValueFontSize )
    {
        doUpdateLayout();
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
QList<caf::PdmOptionItemInfo> RimWellAllocationOverTimePlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options = RimPlot::calculateValueOptions( fieldNeedingOptions );
    if ( !options.empty() )
    {
        return options;
    }

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
        if ( options.empty() )
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
    else if ( m_case && ( fieldNeedingOptions == &m_selectedFromTimeStep || fieldNeedingOptions == &m_selectedToTimeStep ) )
    {
        const QString dateFormatStr = dateFormatString();
        const auto    timeSteps     = m_case->timeStepDates();
        for ( const auto& timeStep : timeSteps )
        {
            options.push_back( caf::PdmOptionItemInfo( timeStep.toString( dateFormatStr ), timeStep ) );
        }
    }
    else if ( m_case && ( fieldNeedingOptions == &m_excludeTimeSteps ) )
    {
        const QString dateFormatStr     = dateFormatString();
        const auto    selectedTimeSteps = getSelectedTimeSteps( m_case->timeStepDates() );
        for ( const auto& timeStep : selectedTimeSteps )
        {
            options.push_back( caf::PdmOptionItemInfo( timeStep.toString( dateFormatStr ), timeStep ) );
        }
    }
    else if ( fieldNeedingOptions == &m_axisTitleFontSize || fieldNeedingOptions == &m_axisValueFontSize )
    {
        options = caf::FontTools::relativeSizeValueOptions( RiaPreferences::current()->defaultPlotFontSize() );
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellAllocationOverTimePlot::getValueTypeText() const
{
    RiaDefines::EclipseUnitSystem        unitSet   = m_case->eclipseCaseData()->unitsType();
    RimWellLogLasFile::WellFlowCondition condition = m_flowDiagSolution ? RimWellLogLasFile::WELL_FLOW_COND_RESERVOIR
                                                                        : RimWellLogLasFile::WELL_FLOW_COND_STANDARD;

    if ( m_flowValueType == FlowValueType::FLOW_RATE_PERCENTAGE )
    {
        QString conditionText = condition == RimWellLogLasFile::WELL_FLOW_COND_RESERVOIR ? "Reservoir" : "Surface";
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
        QString conditionText = condition == RimWellLogLasFile::WELL_FLOW_COND_RESERVOIR ? "Reservoir" : "Surface";
        return QString( "Accumulated %1 Flow Volume Allocation [%]" ).arg( conditionText );
    }

    return QString( "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellAllocationOverTimePlot::dateFormatString() const
{
    return RiaQDateTimeTools::dateFormatString( RiaPreferences::current()->dateFormat(),
                                                RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY );
}

//--------------------------------------------------------------------------------------------------
/// Update selected "From Time Step" and "To Time Step" according to selected case.
/// If both selected time steps exist for case, keep as is, otherwise select first and last time
/// step in case.
//--------------------------------------------------------------------------------------------------
void RimWellAllocationOverTimePlot::setValidTimeStepRangeForCase()
{
    if ( m_case == nullptr || m_case->timeStepDates().empty() )
    {
        return;
    }

    auto isTimeStepInCase = [&]( const QDateTime timeStep ) -> bool
    { return std::find( m_case->timeStepDates().cbegin(), m_case->timeStepDates().cend(), timeStep ) != m_case->timeStepDates().cend(); };
    if ( m_selectedFromTimeStep().isValid() && isTimeStepInCase( m_selectedFromTimeStep() ) && m_selectedToTimeStep().isValid() &&
         isTimeStepInCase( m_selectedToTimeStep() ) )
    {
        return;
    }

    m_selectedFromTimeStep = m_case->timeStepDates().front();
    m_selectedToTimeStep   = m_case->timeStepDates().back();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellAllocationOverTimePlot::axisTitleFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_axisTitleFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellAllocationOverTimePlot::axisValueFontSize() const
{
    return caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_axisValueFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QDateTime> RimWellAllocationOverTimePlot::getSelectedTimeSteps( const std::vector<QDateTime>& timeSteps ) const
{
    const auto timeStepsInRange =
        RiaQDateTimeTools::getTimeStepsWithinSelectedRange( timeSteps, m_selectedFromTimeStep(), m_selectedToTimeStep() );
    return m_timeStepFilterMode == TimeStepFilterMode::TIME_STEP_COUNT
               ? RiaQDateTimeTools::createEvenlyDistributedDates( timeStepsInRange, m_timeStepCount )
               : std::set( timeStepsInRange.begin(), timeStepsInRange.end() );
}
