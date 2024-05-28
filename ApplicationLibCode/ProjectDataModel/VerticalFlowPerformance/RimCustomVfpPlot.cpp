/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024 Equinor ASA
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

#include "RimCustomVfpPlot.h"

#include "RiaColorTables.h"
#include "RiaColorTools.h"
#include "RiaEclipseUnitTools.h"

#include "RigVfpTables.h"

#include "RimPlotAxisProperties.h"
#include "RimPlotCurve.h"
#include "RimVfpDataCollection.h"
#include "RimVfpDefines.h"
#include "RimVfpTable.h"
#include "RimVfpTableData.h"
#include "Tools/RimPlotAxisTools.h"

#include "RiuContextMenuLauncher.h"
#include "RiuPlotCurve.h"
#include "RiuPlotCurveInfoTextProvider.h"
#include "RiuPlotWidget.h"
#include "RiuQwtCurvePointTracker.h"
#include "RiuQwtPlotCurveDefines.h"
#include "RiuQwtPlotWheelZoomer.h"
#include "RiuQwtPlotWidget.h"
#include "RiuQwtPlotZoomer.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include "qwt_plot_panner.h"

//==================================================================================================
//
//
//
//==================================================================================================

CAF_PDM_SOURCE_INIT( RimCustomVfpPlot, "RimCustomVfpPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCustomVfpPlot::RimCustomVfpPlot()
{
    // TODO: add icon
    CAF_PDM_InitObject( "VFP Plot", ":/VfpPlot.svg" );

    CAF_PDM_InitField( &m_plotTitle, "PlotTitle", QString( "VFP Plot" ), "Plot Title" );
    m_plotTitle.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_mainDataSource, "MainDataSouce", "Main VFP Data Source" );

    CAF_PDM_InitFieldNoDefault( &m_additionalDataSources, "AdditionalDataSources", "Additional Data Sources" );
    m_additionalDataSources.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_curveOptionFiltering, "CurveOptionFiltering", "Curve Option Filtering" );
    CAF_PDM_InitFieldNoDefault( &m_curveMatchingType, "CurveMatchingType", "Curve Matching Type" );

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

    CAF_PDM_InitField( &m_flowRateIdx, "LiquidFlowRateIdx", { 0 }, "Flow Rate" );
    m_flowRateIdx.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_thpIdx, "THPIdx", { 0 }, "THP" );
    m_thpIdx.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_articifialLiftQuantityIdx, "ArtificialLiftQuantityIdx", { 0 }, "Artificial Lift Quantity" );
    m_articifialLiftQuantityIdx.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_waterCutIdx, "WaterCutIdx", { 0 }, "Water Cut" );
    m_waterCutIdx.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_gasLiquidRatioIdx, "GasLiquidRatioIdx", { 0 }, "Gas Liquid Ratio" );
    m_gasLiquidRatioIdx.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_familyValues, "FamilyValues", { 0 }, "Family Values" );
    m_familyValues.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

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
    m_plotCurves.uiCapability()->setUiTreeChildrenHidden( true );

    m_showWindow      = true;
    m_showPlotLegends = true;

    setAsPlotMdiWindow();

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCustomVfpPlot::~RimCustomVfpPlot()
{
    removeMdiWindowFromMdiArea();
    deleteViewWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::selectDataSource( RimVfpTable* mainDataSource, const std::vector<RimVfpTable*>& vfpTableData )
{
    m_mainDataSource = mainDataSource;

    m_additionalDataSources.setValue( vfpTableData );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::setTableNumber( int tableNumber )
{
    m_tableNumber = tableNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::initializeObject()
{
    if ( !m_mainDataSource || !m_mainDataSource->dataSource() || !m_mainDataSource->dataSource()->vfpTables() ) return;

    auto vfpTables   = m_mainDataSource->dataSource()->vfpTables();
    auto tableNumber = m_mainDataSource->tableNumber();

    auto table = vfpTables->getTableInitialData( tableNumber );
    initializeFromInitData( table );

    auto values    = vfpTables->getProductionTableData( m_tableNumber(), m_familyVariable() );
    m_familyValues = values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget* RimCustomVfpPlot::plotWidget()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCustomVfpPlot::isCurveHighlightSupported() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::setAutoScaleXEnabled( bool enabled )
{
    m_xAxisProperties->setAutoZoom( enabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::setAutoScaleYEnabled( bool enabled )
{
    m_yAxisProperties->setAutoZoom( enabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::updateAxes()
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
void RimCustomVfpPlot::updateLegend()
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
QString RimCustomVfpPlot::asciiDataForPlotExport() const
{
    return {};
}
/*
    if ( !vfpTables() ) return {};

    auto tableText = vfpTables()->asciiDataForTable( m_tableNumber(),
                                                     m_primaryVariable(),
                                                     m_familyVariable(),
                                                     m_interpolatedVariable(),
                                                     m_flowingPhase(),
                                                     tableSelection() );

    QString wellName;

    if ( m_vfpTableData )
    {
        wellName = m_vfpTableData->name();
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
*/

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::reattachAllCurves()
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
void RimCustomVfpPlot::detachAllCurves()
{
    for ( auto curve : m_plotCurves() )
    {
        curve->detach();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCustomVfpPlot::description() const
{
    return uiName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCustomVfpPlot::infoForCurve( RimPlotCurve* plotCurve ) const
{
    std::vector<RimVfpTable*> tables = m_additionalDataSources.ptrReferencedObjectsByType();
    tables.push_back( m_mainDataSource );

    auto index = m_plotCurves.indexOf( plotCurve );

    size_t curveCount = 0;
    for ( size_t i = 0; i < m_plotData.size(); i++ )
    {
        curveCount += m_plotData[i].size();
        if ( index < curveCount )
        {
            auto tableIndex = i;

            auto    table = tables[tableIndex];
            QString info  = QString( "Table: %1" ).arg( table->tableNumber() );

            auto curveIndex = index - ( curveCount - m_plotData[i].size() );
            auto selection  = tableSelection( table );
            if ( curveIndex < selection.familyValues.size() )
            {
                auto displayValue = convertToDisplayUnit( selection.familyValues[curveIndex], m_familyVariable() );
                auto unitText     = getDisplayUnit( m_familyVariable() );
                auto variableName = m_familyVariable().uiText();

                info += QString( " - %1 %2 %3 " ).arg( variableName ).arg( displayValue ).arg( unitText );
            }

            return info;
        }
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimCustomVfpPlot::viewWidget()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimCustomVfpPlot::snapshotWindowContent()
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
void RimCustomVfpPlot::zoomAll()
{
    setAutoScaleXEnabled( true );
    setAutoScaleYEnabled( true );

    updatePlotWidgetFromAxisRanges();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::onChildrenUpdated( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& updatedObjects )
{
    detachAllCurves();
    reattachAllCurves();

    m_plotWidget->scheduleReplot();
}

//==================================================================================================
//
//
//
//==================================================================================================
class RimVfpCurveInfoTextProvider : public RiuPlotCurveInfoTextProvider
{
public:
    QString curveInfoText( RiuPlotCurve* curve ) const override { return infoForCurve( curve ); };

    QString additionalText( RiuPlotCurve* curve, int sampleIndex ) const override { return {}; }

private:
    QString infoForCurve( RiuPlotCurve* riuCurve ) const
    {
        if ( auto rimcurve = riuCurve->ownerRimCurve() )
        {
            if ( auto owner = rimcurve->firstAncestorOfType<RimCustomVfpPlot>() )
            {
                return owner->infoForCurve( rimcurve );
            }
        }

        return {};
    };
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget* RimCustomVfpPlot::doCreatePlotViewWidget( QWidget* mainWindowParent )
{
    // It seems we risk being called multiple times
    if ( m_plotWidget ) return m_plotWidget;

    auto qwtPlotWidget = new RiuQwtPlotWidget( this, mainWindowParent );
    auto qwtPlot       = qwtPlotWidget->qwtPlot();
    new RiuQwtCurvePointTracker( qwtPlot, true, curveTextProvider() );

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
void RimCustomVfpPlot::deleteViewWidget()
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
void RimCustomVfpPlot::onLoadDataAndUpdate()
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

    detachAllCurves();
    m_plotCurves.deleteChildren();

    int colorIndex = 0;

    std::vector<RimVfpTable*> tables = m_additionalDataSources.ptrReferencedObjectsByType();
    tables.push_back( m_mainDataSource );

    m_plotData.clear();

    for ( const auto& table : tables )
    {
        if ( !table ) continue;

        int  tableIndex = table->tableNumber();
        auto vfpTables  = table->dataSource()->vfpTables();

        auto vfpPlotData = vfpTables->populatePlotData( tableIndex,
                                                        m_primaryVariable(),
                                                        m_familyVariable(),
                                                        m_interpolatedVariable(),
                                                        m_flowingPhase(),
                                                        tableSelection( table ) );

        m_plotData.push_back( vfpPlotData );

        QColor curveColor = RiaColorTables::summaryCurveDefaultPaletteColors().cycledQColor( colorIndex++ );

        populatePlotWidgetWithPlotData( m_plotWidget, vfpPlotData, curveColor );
    }

    updatePlotTitle(
        generatePlotTitle( "Custom", m_tableNumber(), m_tableType(), m_interpolatedVariable(), m_primaryVariable(), m_familyVariable() ) );

    m_plotWidget->setAxisTitleEnabled( RiuPlotAxis::defaultBottom(), true );
    m_plotWidget->setAxisTitleEnabled( RiuPlotAxis::defaultLeft(), true );

    reattachAllCurves();

    updatePlotWidgetFromAxisRanges();

    m_plotWidget->scheduleReplot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::populatePlotWidgetWithPlotData( RiuPlotWidget* plotWidget, const VfpPlotData& plotData, const QColor& color )
{
    plotWidget->setAxisScale( RiuPlotAxis::defaultBottom(), 0, 1 );
    plotWidget->setAxisScale( RiuPlotAxis::defaultLeft(), 0, 1 );
    plotWidget->setAxisAutoScale( RiuPlotAxis::defaultBottom(), true );
    plotWidget->setAxisAutoScale( RiuPlotAxis::defaultLeft(), true );
    plotWidget->setAxisTitleText( RiuPlotAxis::defaultBottom(), plotData.xAxisTitle() );
    plotWidget->setAxisTitleText( RiuPlotAxis::defaultLeft(), plotData.yAxisTitle() );

    for ( auto idx = 0u; idx < plotData.size(); idx++ )
    {
        auto curve = new RimPlotCurve();

        curve->setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID );
        curve->setLineThickness( 2 );
        curve->setColor( RiaColorTools::fromQColorTo3f( color ) );
        curve->setSymbol( RiuPlotCurveSymbol::SYMBOL_ELLIPSE );
        curve->setSymbolSize( 6 );

        curve->setCustomName( plotData.curveTitle( idx ) );
        curve->setParentPlotNoReplot( plotWidget );
        if ( curve->plotCurve() )
        {
            bool useLogarithmicScale = false;
            curve->plotCurve()->setSamplesFromXValuesAndYValues( plotData.xData( idx ), plotData.yData( idx ), useLogarithmicScale );
        }
        curve->updateCurveAppearance();
        curve->appearanceChanged.connect( this, &RimCustomVfpPlot::curveAppearanceChanged );

        m_plotCurves.push_back( curve );
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCustomVfpPlot::axisTitle( RimVfpDefines::ProductionVariableType variableType, RimVfpDefines::FlowingPhaseType flowingPhase )
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
void RimCustomVfpPlot::connectAxisSignals( RimPlotAxisProperties* axis )
{
    axis->settingsChanged.connect( this, &RimCustomVfpPlot::axisSettingsChanged );
    axis->logarithmicChanged.connect( this, &RimCustomVfpPlot::axisLogarithmicChanged );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::axisSettingsChanged( const caf::SignalEmitter* emitter )
{
    updateAxes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::axisLogarithmicChanged( const caf::SignalEmitter* emitter, bool isLogarithmic )
{
    // Currently not supported
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::updatePlotWidgetFromAxisRanges()
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
void RimCustomVfpPlot::updateAxisRangesFromPlotWidget()
{
    RimPlotAxisTools::updateVisibleRangesFromPlotWidget( m_xAxisProperties(), RiuPlotAxis::defaultBottom(), m_plotWidget );
    RimPlotAxisTools::updateVisibleRangesFromPlotWidget( m_yAxisProperties(), RiuPlotAxis::defaultLeft(), m_plotWidget );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::onPlotZoomed()
{
    setAutoScaleXEnabled( false );
    setAutoScaleYEnabled( false );
    updateAxisRangesFromPlotWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::curveAppearanceChanged( const caf::SignalEmitter* emitter )
{
    scheduleReplot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimCustomVfpPlot::availableValues( RimVfpDefines::ProductionVariableType variableType ) const
{
    if ( !m_mainDataSource || !m_mainDataSource->dataSource() || !m_mainDataSource->dataSource()->vfpTables() ) return {};

    auto values = m_mainDataSource->dataSource()->vfpTables()->getProductionTableData( m_tableNumber(), variableType );

    if ( m_curveOptionFiltering() == RimVfpDefines::CurveOptionValuesType::UNION_OF_SELECTED_TABLES )
    {
        std::vector<RimVfpTable*> tables = m_additionalDataSources.ptrReferencedObjectsByType();
        for ( const auto& table : tables )
        {
            if ( !table ) continue;

            auto additionalValues = table->dataSource()->vfpTables()->getProductionTableData( table->tableNumber(), variableType );
            values.insert( values.end(), additionalValues.begin(), additionalValues.end() );
        }

        std::sort( values.begin(), values.end() );
        values.erase( std::unique( values.begin(), values.end() ), values.end() );
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotCurveInfoTextProvider* RimCustomVfpPlot::curveTextProvider()
{
    static auto textProvider = RimVfpCurveInfoTextProvider();
    return &textProvider;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::initializeFromInitData( const VfpTableInitialData& table )
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
double RimCustomVfpPlot::convertToDisplayUnit( double value, RimVfpDefines::ProductionVariableType variableType )
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
void RimCustomVfpPlot::convertToDisplayUnit( std::vector<double>& values, RimVfpDefines::ProductionVariableType variableType )
{
    for ( double& value : values )
        value = convertToDisplayUnit( value, variableType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCustomVfpPlot::getDisplayUnitWithBracket( RimVfpDefines::ProductionVariableType variableType )
{
    QString unit = getDisplayUnit( variableType );
    if ( !unit.isEmpty() ) return QString( "[%1]" ).arg( unit );

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCustomVfpPlot::getDisplayUnit( RimVfpDefines::ProductionVariableType variableType )

{
    if ( variableType == RimVfpDefines::ProductionVariableType::THP ) return "Bar";

    if ( variableType == RimVfpDefines::ProductionVariableType::FLOW_RATE ) return "Sm3/day";

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_mainDataSource );
    uiOrdering.add( &m_additionalDataSources );

    uiOrdering.add( &m_curveMatchingType );
    uiOrdering.add( &m_curveOptionFiltering );

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

        fixedVariablesGroup->add( &m_familyValues );

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
void RimCustomVfpPlot::setFixedVariableUiEditability( caf::PdmFieldHandle& field, RimVfpDefines::ProductionVariableType variableType )
{
    field.uiCapability()->setUiReadOnly( variableType == m_primaryVariable.v() );
    field.uiCapability()->setUiHidden( variableType == m_familyVariable.v() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimCustomVfpPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
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

    else if ( fieldNeedingOptions == &m_familyValues )
    {
        calculateTableValueOptions( m_familyVariable(), options );
    }

    else if ( fieldNeedingOptions == &m_additionalDataSources )
    {
        RimVfpDataCollection* vfpDataCollection = RimVfpDataCollection::instance();
        for ( auto table : vfpDataCollection->vfpTableData() )
        {
            // Exclude main table data object
            if ( table == m_mainDataSource ) continue;

            options.push_back( caf::PdmOptionItemInfo( table->name(), table ) );
        }
    }

    else if ( fieldNeedingOptions == &m_mainDataSource )
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
void RimCustomVfpPlot::calculateTableValueOptions( RimVfpDefines::ProductionVariableType variableType, QList<caf::PdmOptionItemInfo>& options )
{
    auto values = availableValues( variableType );

    for ( size_t i = 0; i < values.size(); i++ )
    {
        options.push_back(
            caf::PdmOptionItemInfo( QString( "%1 %2" ).arg( convertToDisplayUnit( values[i], variableType ) ).arg( getDisplayUnit( variableType ) ),
                                    values[i] ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimPlot::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_mainDataSource )
    {
        initializeObject();
    }

    if ( changedField == &m_familyVariable || changedField == &m_curveOptionFiltering )
    {
        m_familyValues.v() = availableValues( m_familyVariable() );
    }

    loadDataAndUpdate();
    updateLayout();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::initAfterRead()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::updatePlotTitle( const QString& plotTitle )
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
QString RimCustomVfpPlot::generatePlotTitle( const QString&                          wellName,
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
caf::PdmFieldHandle* RimCustomVfpPlot::userDescriptionField()
{
    return &m_plotTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::scheduleReplot()
{
    if ( m_plotWidget )
    {
        m_plotWidget->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
VfpValueSelection RimCustomVfpPlot::tableSelection( RimVfpTable* table ) const
{
    VfpValueSelection selection;

    selection.articifialLiftQuantityValues = m_articifialLiftQuantityIdx();
    selection.flowRateValues               = m_flowRateIdx();
    selection.gasLiquidRatioValues         = m_gasLiquidRatioIdx();
    selection.thpValues                    = m_thpIdx();
    selection.waterCutValues               = m_waterCutIdx();

    if ( m_curveMatchingType() == RimVfpDefines::CurveMatchingType::EXACT )
    {
        selection.familyValues = m_familyValues();
    }
    else if ( m_curveMatchingType() == RimVfpDefines::CurveMatchingType::CLOSEST_MATCH_FAMILY )
    {
        auto familyValues = m_familyValues();
        if ( table && table->dataSource() && table->dataSource()->vfpTables() )
        {
            auto valuesToMatch = table->dataSource()->vfpTables()->getProductionTableData( table->tableNumber(), m_familyVariable() );
            auto indices       = RigVfpTables::uniqueClosestIndices( familyValues, valuesToMatch );
            for ( const auto& i : indices )
            {
                selection.familyValues.push_back( valuesToMatch[i] );
            }
        }
    }

    return selection;
}
