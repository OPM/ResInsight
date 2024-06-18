/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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

#include "RimVfpPlot_deprecated.h"

#include "RiaColorTables.h"
#include "RiaColorTools.h"
#include "RiaEclipseUnitTools.h"
#include "RiaOpmParserTools.h"

#include "RigVfpTables.h"

#include "RimPlotAxisProperties.h"
#include "RimPlotCurve.h"
#include "RimProject.h"
#include "RimVfpDataCollection.h"
#include "RimVfpDefines.h"
#include "RimVfpTable.h"
#include "RimVfpTableData.h"
#include "Tools/RimPlotAxisTools.h"

#include "RiuContextMenuLauncher.h"
#include "RiuPlotCurve.h"
#include "RiuPlotWidget.h"
#include "RiuQwtCurvePointTracker.h"
#include "RiuQwtPlotCurveDefines.h"
#include "RiuQwtPlotWheelZoomer.h"
#include "RiuQwtPlotWidget.h"
#include "RiuQwtPlotZoomer.h"

#include "cafPdmUiComboBoxEditor.h"

#include "qwt_plot_panner.h"

#include <QFileInfo>

#include <memory>

//==================================================================================================
//
//
//
//==================================================================================================

CAF_PDM_SOURCE_INIT( RimVfpPlot_deprecated, "VfpPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpPlot_deprecated::RimVfpPlot_deprecated()
{
    // TODO: add icon
    CAF_PDM_InitObject( "VFP Plot", ":/VfpPlot.svg" );

    CAF_PDM_InitField( &m_plotTitle, "PlotTitle", QString( "VFP Plot" ), "Plot Title" );
    m_plotTitle.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_filePath_OBSOLETE, "FilePath", "File Path" );
    m_filePath_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitFieldNoDefault( &m_vfpTable, "VfpTableData", "VFP Data Source" );

    caf::AppEnum<RimVfpDefines::TableType> defaultTableType = RimVfpDefines::TableType::INJECTION;
    CAF_PDM_InitField( &m_tableType, "TableType", defaultTableType, "Table Type" );
    m_tableType.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_tableNumber, "TableNumber", -1, "Table Number" );
    m_tableNumber.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_referenceDepth, "ReferenceDepth", 0.0, "Reference Depth" );
    m_referenceDepth.uiCapability()->setUiReadOnly( true );

    caf::AppEnum<RimVfpDefines::FlowingPhaseType> defaultFlowingPhase = RimVfpDefines::FlowingPhaseType::WATER;
    CAF_PDM_InitField( &m_flowingPhase, "FlowingPhase", defaultFlowingPhase, "Flowing Phase" );
    m_flowingPhase.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_flowingWaterFraction, "FlowingWaterFraction", "Flowing Water Fraction" );
    m_flowingWaterFraction.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_flowingGasFraction, "FlowingGasFraction", "Flowing Gas Fraction" );
    m_flowingGasFraction.uiCapability()->setUiReadOnly( true );

    caf::AppEnum<RimVfpDefines::InterpolatedVariableType> defaultInterpolatedVariable = RimVfpDefines::InterpolatedVariableType::BHP;
    CAF_PDM_InitField( &m_interpolatedVariable, "InterpolatedVariable", defaultInterpolatedVariable, "Interpolated Variable" );

    caf::AppEnum<RimVfpDefines::ProductionVariableType> defaultPrimaryVariable = RimVfpDefines::ProductionVariableType::FLOW_RATE;
    CAF_PDM_InitField( &m_primaryVariable, "PrimaryVariable", defaultPrimaryVariable, "Primary Variable" );

    caf::AppEnum<RimVfpDefines::ProductionVariableType> defaultFamilyVariable = RimVfpDefines::ProductionVariableType::THP;
    CAF_PDM_InitField( &m_familyVariable, "FamilyVariable", defaultFamilyVariable, "Family Variable" );

    CAF_PDM_InitField( &m_flowRateIdx, "LiquidFlowRateIdx", 0, "Flow Rate" );
    m_flowRateIdx.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_thpIdx, "THPIdx", 0, "THP" );
    m_thpIdx.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_articifialLiftQuantityIdx, "ArtificialLiftQuantityIdx", 0, "Artificial Lift Quantity" );
    m_articifialLiftQuantityIdx.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_waterCutIdx, "WaterCutIdx", 0, "Water Cut" );
    m_waterCutIdx.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_gasLiquidRatioIdx, "GasLiquidRatioIdx", 0, "Gas Liquid Ratio" );
    m_gasLiquidRatioIdx.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_xAxisProperties, "xAxisProperties", "X Axis" );
    m_xAxisProperties = new RimPlotAxisProperties;
    m_xAxisProperties->setNameAndAxis( "X-Axis", "X-Axis", RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM );
    m_xAxisProperties->setEnableTitleTextSettings( false );

    CAF_PDM_InitFieldNoDefault( &m_yAxisProperties, "yAxisProperties", "Y Axis" );
    m_yAxisProperties = new RimPlotAxisProperties;
    m_yAxisProperties->setNameAndAxis( "Y-Axis", "Y-Axis", RiaDefines::PlotAxis::PLOT_AXIS_LEFT );
    m_yAxisProperties->setEnableTitleTextSettings( false );

    connectAxisSignals( m_xAxisProperties() );
    connectAxisSignals( m_yAxisProperties() );

    CAF_PDM_InitFieldNoDefault( &m_plotCurves, "PlotCurves", "Curves" );

    m_showWindow               = true;
    m_showPlotLegends          = true;
    m_dataIsImportedExternally = false;

    setAsPlotMdiWindow();

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpPlot_deprecated::~RimVfpPlot_deprecated()
{
    removeMdiWindowFromMdiArea();
    deleteViewWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::setDataSource( RimVfpTable* vfpTableData )
{
    m_vfpTable = vfpTableData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::setTableNumber( int tableNumber )
{
    m_tableNumber = tableNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::initializeObject()
{
    if ( !vfpTables() ) return;

    auto tableNumber = m_vfpTable->tableNumber();

    // Always use the available table number if only one table is available
    auto prodTableNumbers = vfpTables()->productionTableNumbers();
    auto injTableNumbers  = vfpTables()->injectionTableNumbers();

    if ( prodTableNumbers.size() == 1 && injTableNumbers.empty() )
    {
        tableNumber = prodTableNumbers.front();
    }
    else if ( injTableNumbers.size() == 1 && prodTableNumbers.empty() )
    {
        tableNumber = injTableNumbers.front();
    }

    auto table = vfpTables()->getTableInitialData( tableNumber );
    initializeFromInitData( table );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget* RimVfpPlot_deprecated::plotWidget()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimVfpPlot_deprecated::isCurveHighlightSupported() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::setAutoScaleXEnabled( bool enabled )
{
    m_xAxisProperties->setAutoZoom( enabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::setAutoScaleYEnabled( bool enabled )
{
    m_yAxisProperties->setAutoZoom( enabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::updateAxes()
{
    if ( !m_plotWidget ) return;

    QString title;
    RimPlotAxisTools::updatePlotWidgetFromAxisProperties( m_plotWidget, RiuPlotAxis::defaultBottom(), m_xAxisProperties(), title, {} );
    RimPlotAxisTools::updatePlotWidgetFromAxisProperties( m_plotWidget, RiuPlotAxis::defaultLeft(), m_yAxisProperties(), title, {} );

    m_plotWidget->scheduleReplot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::updateLegend()
{
    if ( !m_plotWidget )
    {
        return;
    }

    // Hide the legend when in multiplot mode, as the legend is handled by the multi plot grid layout
    bool doShowLegend = false;
    if ( isMdiWindow() )
    {
        doShowLegend = m_showPlotLegends;
    }

    if ( doShowLegend )
    {
        m_plotWidget->insertLegend( RiuPlotWidget::Legend::BOTTOM );
    }
    else
    {
        m_plotWidget->clearLegend();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimVfpPlot_deprecated::asciiDataForPlotExport() const
{
    if ( !vfpTables() ) return {};

    auto tableText = vfpTables()->asciiDataForTable( m_tableNumber(),
                                                     m_primaryVariable(),
                                                     m_familyVariable(),
                                                     m_interpolatedVariable(),
                                                     m_flowingPhase(),
                                                     tableSelection() );

    QString wellName;

    if ( m_vfpTable )
    {
        wellName = m_vfpTable->name();
    }
    else
    {
        QString filePath = m_filePath_OBSOLETE.v().path();
        if ( !filePath.isEmpty() )
        {
            QFileInfo fi( filePath );
            QString   wellName = fi.baseName();
        }
    }

    QString plotTitle =
        generatePlotTitle( wellName, m_tableNumber(), m_tableType(), m_interpolatedVariable(), m_primaryVariable(), m_familyVariable() );

    return QString( "%1\n\n%2" ).arg( plotTitle ).arg( tableText );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::reattachAllCurves()
{
    for ( auto curve : m_plotCurves() )
    {
        if ( curve->isChecked() )
        {
            curve->setParentPlotNoReplot( m_plotWidget );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::detachAllCurves()
{
    for ( auto curve : m_plotCurves() )
    {
        curve->detach();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimVfpPlot_deprecated::description() const
{
    return uiName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimVfpPlot_deprecated::viewWidget()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimVfpPlot_deprecated::snapshotWindowContent()
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
void RimVfpPlot_deprecated::zoomAll()
{
    setAutoScaleXEnabled( true );
    setAutoScaleYEnabled( true );

    updatePlotWidgetFromAxisRanges();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::setDataIsImportedExternally( bool dataIsImportedExternally )
{
    m_dataIsImportedExternally = dataIsImportedExternally;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimVfpPlot_deprecated::tableNumber() const
{
    return m_tableNumber();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::onChildrenUpdated( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& updatedObjects )
{
    detachAllCurves();
    reattachAllCurves();

    m_plotWidget->scheduleReplot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget* RimVfpPlot_deprecated::doCreatePlotViewWidget( QWidget* mainWindowParent )
{
    // It seems we risk being called multiple times
    if ( m_plotWidget ) return m_plotWidget;

    auto qwtPlotWidget = new RiuQwtPlotWidget( this, mainWindowParent );
    auto qwtPlot       = qwtPlotWidget->qwtPlot();
    new RiuQwtCurvePointTracker( qwtPlot, true, nullptr );

    // LeftButton for the zooming
    auto plotZoomer = new RiuQwtPlotZoomer( qwtPlot->canvas() );
    plotZoomer->setTrackerMode( QwtPicker::AlwaysOff );
    plotZoomer->initMousePattern( 1 );

    // MidButton for the panning
    auto panner = new QwtPlotPanner( qwtPlot->canvas() );
    panner->setMouseButton( Qt::MiddleButton );

    auto wheelZoomer = new RiuQwtPlotWheelZoomer( qwtPlot );

    // Use lambda functions to connect signals to functions instead of slots
    connect( wheelZoomer, &RiuQwtPlotWheelZoomer::zoomUpdated, [=, this]() { onPlotZoomed(); } );
    connect( plotZoomer, &RiuQwtPlotZoomer::zoomed, [=, this]() { onPlotZoomed(); } );
    connect( panner, &QwtPlotPanner::panned, [=, this]() { onPlotZoomed(); } );
    connect( qwtPlotWidget, &RiuQwtPlotWidget::plotZoomed, [=, this]() { onPlotZoomed(); } );

    // Remove event filter to disable unwanted highlighting on left click in plot.
    qwtPlotWidget->removeEventFilter();

    new RiuContextMenuLauncher( qwtPlotWidget, { "RicShowPlotDataFeature" } );

    m_plotWidget = qwtPlotWidget;

    updateLegend();
    onLoadDataAndUpdate();

    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::deleteViewWidget()
{
    if ( m_plotWidget )
    {
        m_plotWidget->setParent( nullptr );
        delete m_plotWidget;
        m_plotWidget = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::onLoadDataAndUpdate()
{
    if ( isMdiWindow() )
    {
        updateMdiWindowVisibility();
    }
    else
    {
        updateParentLayout();
    }

    if ( !m_plotWidget )
    {
        return;
    }

    updateLegend();

    QString wellName;

    if ( vfpTables() )
    {
        wellName = vfpTableData()->baseFileName();

        auto vfpPlotData = vfpTables()->populatePlotData( m_tableNumber(),
                                                          m_primaryVariable(),
                                                          m_familyVariable(),
                                                          m_interpolatedVariable(),
                                                          m_flowingPhase(),
                                                          tableSelection() );

        populatePlotWidgetWithPlotData( m_plotWidget, vfpPlotData );
    }

    updatePlotTitle(
        generatePlotTitle( wellName, m_tableNumber(), m_tableType(), m_interpolatedVariable(), m_primaryVariable(), m_familyVariable() ) );

    m_plotWidget->setAxisTitleEnabled( RiuPlotAxis::defaultBottom(), true );
    m_plotWidget->setAxisTitleEnabled( RiuPlotAxis::defaultLeft(), true );

    reattachAllCurves();

    updatePlotWidgetFromAxisRanges();

    m_plotWidget->scheduleReplot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::populatePlotWidgetWithPlotData( RiuPlotWidget* plotWidget, const VfpPlotData& plotData )
{
    plotWidget->setAxisScale( RiuPlotAxis::defaultBottom(), 0, 1 );
    plotWidget->setAxisScale( RiuPlotAxis::defaultLeft(), 0, 1 );
    plotWidget->setAxisAutoScale( RiuPlotAxis::defaultBottom(), true );
    plotWidget->setAxisAutoScale( RiuPlotAxis::defaultLeft(), true );
    plotWidget->setAxisTitleText( RiuPlotAxis::defaultBottom(), plotData.xAxisTitle() );
    plotWidget->setAxisTitleText( RiuPlotAxis::defaultLeft(), plotData.yAxisTitle() );

    if ( m_plotCurves.size() != plotData.size() )
    {
        detachAllCurves();
        m_plotCurves.deleteChildren();

        for ( auto idx = 0u; idx < plotData.size(); idx++ )
        {
            QColor qtClr = RiaColorTables::summaryCurveDefaultPaletteColors().cycledQColor( idx );

            auto curve = new RimPlotCurve();

            curve->setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID );
            curve->setLineThickness( 2 );
            curve->setColor( RiaColorTools::fromQColorTo3f( qtClr ) );
            curve->setSymbol( RiuPlotCurveSymbol::SYMBOL_ELLIPSE );
            curve->setSymbolSize( 6 );

            m_plotCurves.push_back( curve );
        }

        updateConnectedEditors();
    }

    auto plotCurves = m_plotCurves.childrenByType();

    for ( auto idx = 0u; idx < plotData.size(); idx++ )
    {
        auto curve = plotCurves[idx];
        if ( !curve ) continue;

        curve->setCustomName( plotData.curveTitle( idx ) );
        curve->setParentPlotNoReplot( plotWidget );
        if ( curve->plotCurve() )
        {
            bool useLogarithmicScale = false;
            curve->plotCurve()->setSamplesFromXValuesAndYValues( plotData.xData( idx ), plotData.yData( idx ), useLogarithmicScale );
        }
        curve->updateCurveAppearance();
        curve->appearanceChanged.connect( this, &RimVfpPlot_deprecated::curveAppearanceChanged );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimVfpPlot_deprecated::axisTitle( RimVfpDefines::ProductionVariableType variableType, RimVfpDefines::FlowingPhaseType flowingPhase )
{
    QString title;

    if ( flowingPhase == RimVfpDefines::FlowingPhaseType::GAS )
    {
        title = "Gas ";
    }
    else
    {
        title = "Liquid ";
    }
    title += QString( "%1 %2" ).arg( caf::AppEnum<RimVfpDefines::ProductionVariableType>::uiText( variableType ),
                                     getDisplayUnitWithBracket( variableType ) );

    return title;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::connectAxisSignals( RimPlotAxisProperties* axis )
{
    axis->settingsChanged.connect( this, &RimVfpPlot_deprecated::axisSettingsChanged );
    axis->logarithmicChanged.connect( this, &RimVfpPlot_deprecated::axisLogarithmicChanged );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::axisSettingsChanged( const caf::SignalEmitter* emitter )
{
    updateAxes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::axisLogarithmicChanged( const caf::SignalEmitter* emitter, bool isLogarithmic )
{
    // Currently not supported
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::updatePlotWidgetFromAxisRanges()
{
    if ( m_plotWidget )
    {
        updateAxes();

        if ( auto qwtWidget = dynamic_cast<RiuQwtPlotWidget*>( m_plotWidget.data() ) )
        {
            if ( qwtWidget->qwtPlot() ) qwtWidget->qwtPlot()->updateAxes();
        }

        updateAxisRangesFromPlotWidget();
        m_plotWidget->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::updateAxisRangesFromPlotWidget()
{
    RimPlotAxisTools::updateVisibleRangesFromPlotWidget( m_xAxisProperties(), RiuPlotAxis::defaultBottom(), m_plotWidget );
    RimPlotAxisTools::updateVisibleRangesFromPlotWidget( m_yAxisProperties(), RiuPlotAxis::defaultLeft(), m_plotWidget );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::onPlotZoomed()
{
    setAutoScaleXEnabled( false );
    setAutoScaleYEnabled( false );
    updateAxisRangesFromPlotWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::curveAppearanceChanged( const caf::SignalEmitter* emitter )
{
    scheduleReplot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::initializeFromInitData( const VfpTableInitialData& table )
{
    m_tableType            = table.isProductionTable ? RimVfpDefines::TableType::PRODUCTION : RimVfpDefines::TableType::INJECTION;
    m_tableNumber          = table.tableNumber;
    m_referenceDepth       = table.datumDepth;
    m_flowingPhase         = table.flowingPhase;
    m_flowingGasFraction   = table.gasFraction;
    m_flowingWaterFraction = table.waterFraction;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpTableData* RimVfpPlot_deprecated::vfpTableData() const
{
    if ( m_vfpTable ) return m_vfpTable->dataSource();

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigVfpTables* RimVfpPlot_deprecated::vfpTables() const
{
    if ( vfpTableData() )
    {
        vfpTableData()->ensureDataIsImported();
        return vfpTableData()->vfpTables();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimVfpPlot_deprecated::convertToDisplayUnit( double value, RimVfpDefines::ProductionVariableType variableType )
{
    if ( variableType == RimVfpDefines::ProductionVariableType::THP )
    {
        return RiaEclipseUnitTools::pascalToBar( value );
    }

    if ( variableType == RimVfpDefines::ProductionVariableType::FLOW_RATE )
    {
        // Convert to m3/sec to m3/day
        return value * static_cast<double>( 24 * 60 * 60 );
    }

    return value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::convertToDisplayUnit( std::vector<double>& values, RimVfpDefines::ProductionVariableType variableType )
{
    for ( double& value : values )
        value = convertToDisplayUnit( value, variableType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimVfpPlot_deprecated::getDisplayUnitWithBracket( RimVfpDefines::ProductionVariableType variableType )
{
    QString unit = getDisplayUnit( variableType );
    if ( !unit.isEmpty() ) return QString( "[%1]" ).arg( unit );

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimVfpPlot_deprecated::getDisplayUnit( RimVfpDefines::ProductionVariableType variableType )

{
    if ( variableType == RimVfpDefines::ProductionVariableType::THP ) return "Bar";

    if ( variableType == RimVfpDefines::ProductionVariableType::FLOW_RATE ) return "Sm3/day";

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_vfpTable );

    uiOrdering.add( &m_tableType );
    uiOrdering.add( &m_tableNumber );
    uiOrdering.add( &m_referenceDepth );
    uiOrdering.add( &m_interpolatedVariable );
    uiOrdering.add( &m_flowingPhase );

    if ( m_tableType == RimVfpDefines::TableType::PRODUCTION )
    {
        uiOrdering.add( &m_flowingWaterFraction );
        uiOrdering.add( &m_flowingGasFraction );

        uiOrdering.add( &m_primaryVariable );
        uiOrdering.add( &m_familyVariable );

        caf::PdmUiOrdering* fixedVariablesGroup = uiOrdering.addNewGroup( "Fixed Variables" );
        fixedVariablesGroup->add( &m_flowRateIdx );
        fixedVariablesGroup->add( &m_thpIdx );
        fixedVariablesGroup->add( &m_articifialLiftQuantityIdx );
        fixedVariablesGroup->add( &m_waterCutIdx );
        fixedVariablesGroup->add( &m_gasLiquidRatioIdx );

        // Disable the choices for variables as primary or family
        setFixedVariableUiEditability( m_flowRateIdx, RimVfpDefines::ProductionVariableType::FLOW_RATE );
        setFixedVariableUiEditability( m_thpIdx, RimVfpDefines::ProductionVariableType::THP );
        setFixedVariableUiEditability( m_articifialLiftQuantityIdx, RimVfpDefines::ProductionVariableType::ARTIFICIAL_LIFT_QUANTITY );
        setFixedVariableUiEditability( m_waterCutIdx, RimVfpDefines::ProductionVariableType::WATER_CUT );
        setFixedVariableUiEditability( m_gasLiquidRatioIdx, RimVfpDefines::ProductionVariableType::GAS_LIQUID_RATIO );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::setFixedVariableUiEditability( caf::PdmField<int>& field, RimVfpDefines::ProductionVariableType variableType )
{
    field.uiCapability()->setUiReadOnly( variableType == m_primaryVariable.v() || variableType == m_familyVariable.v() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimVfpPlot_deprecated::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options = RimPlot::calculateValueOptions( fieldNeedingOptions );

    if ( fieldNeedingOptions == &m_flowRateIdx )
    {
        calculateTableValueOptions( RimVfpDefines::ProductionVariableType::FLOW_RATE, options );
    }

    else if ( fieldNeedingOptions == &m_thpIdx )
    {
        calculateTableValueOptions( RimVfpDefines::ProductionVariableType::THP, options );
    }

    else if ( fieldNeedingOptions == &m_articifialLiftQuantityIdx )
    {
        calculateTableValueOptions( RimVfpDefines::ProductionVariableType::ARTIFICIAL_LIFT_QUANTITY, options );
    }

    else if ( fieldNeedingOptions == &m_waterCutIdx )
    {
        calculateTableValueOptions( RimVfpDefines::ProductionVariableType::WATER_CUT, options );
    }

    else if ( fieldNeedingOptions == &m_gasLiquidRatioIdx )
    {
        calculateTableValueOptions( RimVfpDefines::ProductionVariableType::GAS_LIQUID_RATIO, options );
    }

    else if ( fieldNeedingOptions == &m_vfpTable )
    {
        RimVfpDataCollection* vfpDataCollection = RimVfpDataCollection::instance();
        for ( auto table : vfpDataCollection->vfpTableData() )
        {
            options.push_back( caf::PdmOptionItemInfo( table->name(), table ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::calculateTableValueOptions( RimVfpDefines::ProductionVariableType variableType,
                                                        QList<caf::PdmOptionItemInfo>&        options )
{
    if ( vfpTables() )
    {
        auto values = vfpTables()->getProductionTableData( m_tableNumber(), variableType );

        for ( size_t i = 0; i < values.size(); i++ )
        {
            options.push_back(
                caf::PdmOptionItemInfo( QString( "%1 %2" ).arg( convertToDisplayUnit( values[i], variableType ) ).arg( getDisplayUnit( variableType ) ),
                                        static_cast<int>( i ) ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimPlot::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_vfpTable )
    {
        initializeObject();
    }

    loadDataAndUpdate();
    updateLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::initAfterRead()
{
    auto filePath = m_filePath_OBSOLETE.v().path();
    if ( filePath.isEmpty() ) return;

    auto vfpDataCollection = RimVfpDataCollection::instance();
    if ( vfpDataCollection )
    {
        auto tableData = vfpDataCollection->appendTableDataObject( filePath );
        if ( tableData )
        {
            setDataSource( tableData );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::updatePlotTitle( const QString& plotTitle )
{
    m_plotTitle = plotTitle;

    updateMdiWindowTitle();

    if ( m_plotWidget )
    {
        m_plotWidget->setPlotTitle( plotTitle );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimVfpPlot_deprecated::generatePlotTitle( const QString&                          wellName,
                                                  int                                     tableNumber,
                                                  RimVfpDefines::TableType                tableType,
                                                  RimVfpDefines::InterpolatedVariableType interpolatedVariable,
                                                  RimVfpDefines::ProductionVariableType   primaryVariable,
                                                  RimVfpDefines::ProductionVariableType   familyVariable )
{
    QString tableTypeText            = caf::AppEnum<RimVfpDefines::TableType>::uiText( tableType );
    QString interpolatedVariableText = caf::AppEnum<RimVfpDefines::InterpolatedVariableType>::uiText( interpolatedVariable );
    QString primaryVariableText      = caf::AppEnum<RimVfpDefines::ProductionVariableType>::uiText( primaryVariable );
    QString plotTitleStr =
        QString( "VFP: %1 (%2) #%3 - %4 x %5" ).arg( wellName ).arg( tableTypeText ).arg( tableNumber ).arg( interpolatedVariableText ).arg( primaryVariableText );

    return plotTitleStr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimVfpPlot_deprecated::userDescriptionField()
{
    return &m_plotTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot_deprecated::scheduleReplot()
{
    if ( m_plotWidget )
    {
        m_plotWidget->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VfpTableSelection RimVfpPlot_deprecated::tableSelection() const
{
    return { m_flowRateIdx(), m_thpIdx(), m_articifialLiftQuantityIdx(), m_waterCutIdx(), m_gasLiquidRatioIdx() };
}
