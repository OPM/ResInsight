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
#include "RiaPreferences.h"

#include "RifCsvDataTableFormatter.h"

#include "RigVfpTables.h"

#include "RimColorLegend.h"
#include "RimColorLegendItem.h"
#include "RimPlotAxisProperties.h"
#include "RimPlotCurve.h"
#include "RimRegularLegendConfig.h"
#include "RimVfpDataCollection.h"
#include "RimVfpDefines.h"
#include "RimVfpTable.h"
#include "RimVfpTableData.h"
#include "Tools/RimPlotAxisTools.h"

#include "RiuAbstractLegendFrame.h"
#include "RiuContextMenuLauncher.h"
#include "RiuDraggableOverlayFrame.h"
#include "RiuPlotCurve.h"
#include "RiuPlotCurveInfoTextProvider.h"
#include "RiuPlotWidget.h"
#include "RiuQwtCurvePointTracker.h"
#include "RiuQwtPlotCurveDefines.h"
#include "RiuQwtPlotWheelZoomer.h"
#include "RiuQwtPlotWidget.h"
#include "RiuQwtPlotZoomer.h"
#include "RiuTools.h"

#include "cafColorTable.h"
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

    CAF_PDM_InitFieldNoDefault( &m_comparisonTables, "ComparisonTables", "Comparison Tables" );
    m_comparisonTables.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_curveValueOptions, "CurveValueOptions", "Curve Value Options" );
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

    CAF_PDM_InitFieldNoDefault( &m_flowRate, "LiquidFlowRate", "Flow Rate" );
    m_flowRate.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_thp, "THP", "THP" );
    m_thp.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_artificialLiftQuantity, "ArtificialLiftQuantity", "Artificial Lift Quantity" );
    m_artificialLiftQuantity.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_waterCut, "WaterCut", "Water Cut" );
    m_waterCut.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_gasLiquidRatio, "GasLiquidRatio", "Gas Liquid Ratio" );
    m_gasLiquidRatio.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_xAxisProperties, "xAxisProperties", "X Axis" );
    m_xAxisProperties = new RimPlotAxisProperties;
    m_xAxisProperties->setNameAndAxis( "X-Axis", "X-Axis", RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM );
    m_xAxisProperties->configureForBasicUse();

    CAF_PDM_InitFieldNoDefault( &m_yAxisProperties, "yAxisProperties", "Y Axis" );
    m_yAxisProperties = new RimPlotAxisProperties;
    m_yAxisProperties->setNameAndAxis( "Y-Axis", "Y-Axis", RiaDefines::PlotAxis::PLOT_AXIS_LEFT );
    m_yAxisProperties->configureForBasicUse();

    CAF_PDM_InitField( &m_curveThickness, "CurveThickness", 2, "Line Thickness" );
    m_curveThickness.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_curveSymbolSize, "CurveSymbolSize", 10, "Symbol Size" );

    connectAxisSignals( m_xAxisProperties() );
    connectAxisSignals( m_yAxisProperties() );

    CAF_PDM_InitFieldNoDefault( &m_plotCurves, "PlotCurves", "Curves" );
    m_plotCurves.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_colorLegend, "ColorLegend", "" );
    m_colorLegend = new RimColorLegend();
    m_colorLegend->setColorLegendName( "Curve Colors" );
    m_colorLegend->changed.connect( this, &RimCustomVfpPlot::legendColorsChanged );

    CAF_PDM_InitFieldNoDefault( &m_legendConfig, "LegendConfig", "" );
    m_legendConfig = new RimRegularLegendConfig();
    m_legendConfig->setMappingMode( RimRegularLegendConfig::MappingType::CATEGORY_INTEGER );
    m_legendConfig->setColorLegend( m_colorLegend );
    m_legendConfig->uiCapability()->setUiTreeHidden( true );

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

    m_comparisonTables.setValue( vfpTableData );
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
}

//--------------------------------------------------------------------------------------------------
///
void RimCustomVfpPlot::initializeSelection()
{
    std::map<RimVfpDefines::ProductionVariableType, caf::PdmField<std::vector<double>>*> typeAndField =
        { { RimVfpDefines::ProductionVariableType::FLOW_RATE, &m_flowRate },
          { RimVfpDefines::ProductionVariableType::THP, &m_thp },
          { RimVfpDefines::ProductionVariableType::ARTIFICIAL_LIFT_QUANTITY, &m_artificialLiftQuantity },
          { RimVfpDefines::ProductionVariableType::WATER_CUT, &m_waterCut },
          { RimVfpDefines::ProductionVariableType::GAS_LIQUID_RATIO, &m_gasLiquidRatio } };

    for ( const auto& [variableType, field] : typeAndField )
    {
        auto values = availableValues( variableType );
        if ( m_familyVariable() == variableType )
            field->v() = values;
        else if ( !values.empty() )
            field->v() = { values.front() };
        else
            field->v() = {};
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::createDefaultColors()
{
    auto colors = RiaColorTables::curveSetPaletteColors();

    int colorIndex = 1;
    for ( auto color : colors.color3fArray() )
    {
        auto* colorLegendItem = new RimColorLegendItem;
        colorLegendItem->setValues( QString( "Color %1" ).arg( colorIndex ), colorIndex, color );

        m_colorLegend->appendColorLegendItem( colorLegendItem );

        colorIndex++;
    }

    m_legendConfig->setColorLegend( m_colorLegend );

    setColorItemCategoryHidden();
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

    RimPlotAxisTools::updatePlotWidgetFromAxisProperties( m_plotWidget, RiuPlotAxis::defaultBottom(), m_xAxisProperties(), m_xAxisTitle, {} );
    RimPlotAxisTools::updatePlotWidgetFromAxisProperties( m_plotWidget, RiuPlotAxis::defaultLeft(), m_yAxisProperties(), m_yAxisTitle, {} );

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
    QString asciiData;

    size_t plotCurveIdx = 0;

    for ( const auto& curveData : m_plotData )
    {
        for ( size_t curveIdx = 0; curveIdx < curveData.size(); curveIdx++ )
        {
            asciiData += curveData.curveTitle( curveIdx );

            if ( !m_comparisonTables.empty() && plotCurveIdx < m_plotCurveMetaData.size() )
            {
                auto plotCurveData = m_plotCurveMetaData[plotCurveIdx];
                asciiData += QString( " (Table: %1)" ).arg( plotCurveData.tableNumber );
            }

            asciiData += "\n";

            QTextStream              stream( &asciiData );
            RifCsvDataTableFormatter formatter( stream, RiaPreferences::current()->csvTextExportFieldSeparator );

            std::vector<RifTextDataTableColumn> header;
            const int                           precision = 2;
            header.emplace_back( curveData.xAxisTitle(), RifTextDataTableDoubleFormatting( RIF_FLOAT, precision ) );
            header.emplace_back( curveData.yAxisTitle(), RifTextDataTableDoubleFormatting( RIF_FLOAT, precision ) );
            formatter.header( header );

            for ( size_t i = 0; i < curveData.xData( curveIdx ).size(); i++ )
            {
                formatter.add( curveData.xData( curveIdx )[i] );
                formatter.add( curveData.yData( curveIdx )[i] );
                formatter.rowCompleted();
            }
            formatter.tableCompleted();

            plotCurveIdx++;
        }
        asciiData += "\n";
    }

    return asciiData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::reattachAllCurves()
{
    for ( const auto& curve : m_plotCurves() )
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
    for ( const auto& curve : m_plotCurves() )
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
    auto plotCurveIdx = m_plotCurves.indexOf( plotCurve );

    if ( plotCurveIdx < m_plotCurveMetaData.size() )
    {
        auto    values = m_plotCurveMetaData[plotCurveIdx];
        QString info   = QString( "Table: %1\n" ).arg( values.tableNumber );
        info += values.curveName;

        if ( m_familyVariable() != RimVfpDefines::ProductionVariableType::WATER_CUT && m_waterCut().size() > 1 )
            info += QString( "\nWC: %1 %2" )
                        .arg( convertToDisplayUnit( values.waterCutValue, RimVfpDefines::ProductionVariableType::WATER_CUT ) )
                        .arg( getDisplayUnitWithBracket( RimVfpDefines::ProductionVariableType::WATER_CUT ) );

        if ( m_familyVariable() != RimVfpDefines::ProductionVariableType::GAS_LIQUID_RATIO && m_gasLiquidRatio().size() > 1 )
            info += QString( "\nGLR: %1 %2" )
                        .arg( convertToDisplayUnit( values.gasLiquidRatioValue, RimVfpDefines::ProductionVariableType::GAS_LIQUID_RATIO ) )
                        .arg( getDisplayUnitWithBracket( RimVfpDefines::ProductionVariableType::GAS_LIQUID_RATIO ) );

        if ( m_familyVariable() != RimVfpDefines::ProductionVariableType::ARTIFICIAL_LIFT_QUANTITY && m_artificialLiftQuantity().size() > 1 )
            info += QString( "\nLift: %1 %2" )
                        .arg( convertToDisplayUnit( values.artificialLiftQuantityValue,
                                                    RimVfpDefines::ProductionVariableType::ARTIFICIAL_LIFT_QUANTITY ) )
                        .arg( getDisplayUnitWithBracket( RimVfpDefines::ProductionVariableType::ARTIFICIAL_LIFT_QUANTITY ) );

        if ( m_familyVariable() != RimVfpDefines::ProductionVariableType::THP && m_thp().size() > 1 )
            info += QString( "\nTPH: %1 %2" )
                        .arg( convertToDisplayUnit( values.thpValue, RimVfpDefines::ProductionVariableType::THP ) )
                        .arg( getDisplayUnitWithBracket( RimVfpDefines::ProductionVariableType::THP ) );

        if ( m_familyVariable() != RimVfpDefines::ProductionVariableType::FLOW_RATE && m_flowRate().size() > 1 )
            info += QString( "\nRate: %1 %2" )
                        .arg( convertToDisplayUnit( values.flowRateValue, RimVfpDefines::ProductionVariableType::FLOW_RATE ) )
                        .arg( getDisplayUnitWithBracket( RimVfpDefines::ProductionVariableType::FLOW_RATE ) );

        info += "\n";

        return info;
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
/// Create all possible combinations of the vectors passed in.
//--------------------------------------------------------------------------------------------------
void generateCombinations( const std::vector<std::vector<double>>& vectors,
                           std::vector<double>&                    currentCombination,
                           std::vector<std::vector<double>>&       allCombinations,
                           size_t                                  depth )
{
    if ( depth == vectors.size() )
    {
        allCombinations.push_back( currentCombination );
        return;
    }

    for ( const auto& value : vectors[depth] )
    {
        currentCombination[depth] = value;
        generateCombinations( vectors, currentCombination, allCombinations, depth + 1 );
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

    std::vector<RimVfpTable*> tables;
    tables.push_back( m_mainDataSource );
    auto comparisonTables = m_comparisonTables.ptrReferencedObjectsByType();
    std::copy( comparisonTables.begin(), comparisonTables.end(), std::back_inserter( tables ) );

    m_plotData.clear();
    m_plotCurveMetaData.clear();

    size_t           curveSetCount = 0;
    CurveNameContent curveNameContent;

    for ( const auto& table : tables )
    {
        if ( !table ) continue;

        int  tableNumber = table->tableNumber();
        auto vfpTables   = table->dataSource()->vfpTables();

        if ( table->tableType() == RimVfpDefines::TableType::INJECTION )
        {
            auto vfpPlotData = vfpTables->populatePlotData( tableNumber,
                                                            m_primaryVariable(),
                                                            m_familyVariable(),
                                                            m_interpolatedVariable(),
                                                            m_flowingPhase(),
                                                            VfpTableSelection() );

            m_plotData.push_back( vfpPlotData );

            QColor curveColor = curveColors().cycledQColor( colorIndex );

            curveNameContent.defaultName = true;
            populatePlotWidgetWithPlotData( m_plotWidget, vfpPlotData, VfpValueSelection(), tableNumber, curveColor, curveNameContent );
            colorIndex++;

            curveSetCount += 1;
        }
        else
        {
            auto valueSelections = computeValueSelectionCombinations();
            curveSetCount += valueSelections.size();

            if ( tables.size() > 1 ) curveNameContent.tableNumber = true;
            if ( m_flowRate().size() > 1 ) curveNameContent.flowRate = true;
            if ( m_thp().size() > 1 ) curveNameContent.thp = true;
            if ( m_artificialLiftQuantity().size() > 1 ) curveNameContent.artificialLiftQuantity = true;
            if ( m_waterCut().size() > 1 ) curveNameContent.waterCut = true;
            if ( m_gasLiquidRatio().size() > 1 ) curveNameContent.gasLiquidRatio = true;

            for ( auto& valueSelection : valueSelections )
            {
                valueSelection.familyValues = familyValuesForTable( table );

                auto vfpPlotData = vfpTables->populatePlotData( tableNumber,
                                                                m_primaryVariable(),
                                                                m_familyVariable(),
                                                                m_interpolatedVariable(),
                                                                m_flowingPhase(),
                                                                valueSelection );

                m_plotData.push_back( vfpPlotData );

                QColor curveColor = curveColors().cycledQColor( colorIndex );

                populatePlotWidgetWithPlotData( m_plotWidget, vfpPlotData, valueSelection, tableNumber, curveColor, curveNameContent );
                colorIndex++;
            }
        }
    }

    updatePlotTitle(
        generatePlotTitle( "", m_tableNumber(), m_tableType(), m_interpolatedVariable(), m_primaryVariable(), m_familyVariable() ) );

    updateLegendWidget( curveSetCount, curveNameContent );

    m_plotWidget->setAxisTitleEnabled( RiuPlotAxis::defaultBottom(), true );
    m_plotWidget->setAxisTitleEnabled( RiuPlotAxis::defaultLeft(), true );

    reattachAllCurves();

    updatePlotWidgetFromAxisRanges();

    m_plotWidget->scheduleReplot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::initAfterRead()
{
    setColorItemCategoryHidden();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::updateLegendWidget( size_t curveSetCount, CurveNameContent& curveNameContent )
{
    if ( !m_plotWidget ) return;

    if ( !m_legendOverlayFrame )
    {
        m_legendOverlayFrame = new RiuDraggableOverlayFrame( m_plotWidget->getParentForOverlay(), m_plotWidget->overlayMargins() );
    }

    if ( curveSetCount > 1 )
    {
        size_t                                               plotCurveIdx = 0;
        std::vector<std::tuple<QString, int, cvf::Color3ub>> categories;
        for ( size_t i = 0; i < curveSetCount; i++ )
        {
            auto color = curveColors().cycledColor3f( i );

            auto formatNamePart = [&]( RimVfpDefines::ProductionVariableType variableType, double selectionValue, const QString& namePart ) -> QString
            {
                double displayValue = convertToDisplayUnit( selectionValue, variableType );
                return QString( " %1:%2" ).arg( namePart ).arg( displayValue );
            };

            if ( plotCurveIdx < m_plotCurveMetaData.size() )
            {
                auto plotData = m_plotCurveMetaData[plotCurveIdx];

                QString curveSetName;
                if ( curveNameContent.tableNumber ) curveSetName += QString( " Table:%1" ).arg( plotData.tableNumber );

                using pvt = RimVfpDefines::ProductionVariableType;

                if ( curveNameContent.thp && m_familyVariable() != pvt::THP )
                {
                    curveSetName += formatNamePart( pvt::THP, plotData.thpValue, "THP" );
                }
                if ( curveNameContent.gasLiquidRatio && m_familyVariable() != pvt::GAS_LIQUID_RATIO )
                {
                    curveSetName += formatNamePart( pvt::GAS_LIQUID_RATIO, plotData.gasLiquidRatioValue, "GLR" );
                }
                if ( curveNameContent.waterCut && m_familyVariable() != pvt::WATER_CUT )
                {
                    curveSetName += formatNamePart( pvt::WATER_CUT, plotData.waterCutValue, "WC" );
                }
                if ( curveNameContent.artificialLiftQuantity && m_familyVariable() != pvt::ARTIFICIAL_LIFT_QUANTITY )
                {
                    curveSetName += formatNamePart( pvt::ARTIFICIAL_LIFT_QUANTITY, plotData.artificialLiftQuantityValue, "Lift" );
                }
                if ( curveNameContent.flowRate && m_familyVariable() != pvt::FLOW_RATE )
                {
                    curveSetName += formatNamePart( pvt::FLOW_RATE, plotData.flowRateValue, "Rate" );
                }

                categories.push_back( std::make_tuple( curveSetName, static_cast<int>( i ), cvf::Color3ub( color ) ) );
            }

            plotCurveIdx += m_plotData[i].size();
        }

        // Reverse the categories to make the first curve the topmost in the legend
        std::reverse( categories.begin(), categories.end() );
        m_legendConfig->setCategoryItems( categories );

        m_legendOverlayFrame->setContentFrame( m_legendConfig->makeLegendFrame() );
        m_plotWidget->addOverlayFrame( m_legendOverlayFrame );
    }
    else
    {
        m_plotWidget->removeOverlayFrame( m_legendOverlayFrame );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::populatePlotWidgetWithPlotData( RiuPlotWidget*           plotWidget,
                                                       const VfpPlotData&       plotData,
                                                       const VfpValueSelection& valueSelection,
                                                       int                      tableNumber,
                                                       const QColor&            color,
                                                       const CurveNameContent&  curveNameContent )
{
    if ( !plotWidget ) return;

    plotWidget->setAxisScale( RiuPlotAxis::defaultBottom(), 0, 1 );
    plotWidget->setAxisScale( RiuPlotAxis::defaultLeft(), 0, 1 );
    plotWidget->setAxisAutoScale( RiuPlotAxis::defaultBottom(), true );
    plotWidget->setAxisAutoScale( RiuPlotAxis::defaultLeft(), true );

    m_xAxisTitle = plotData.xAxisTitle();
    m_yAxisTitle = plotData.yAxisTitle();

    auto formatCurveNamePart =
        [&]( RimVfpDefines::ProductionVariableType variableType, double familyValue, double selectionValue, const QString& namePart ) -> QString
    {
        double value        = ( variableType == m_familyVariable() ) ? familyValue : selectionValue;
        double displayValue = convertToDisplayUnit( value, variableType );
        return QString( " %1:%2" ).arg( namePart ).arg( displayValue );
    };

    for ( auto curveIndex = 0u; curveIndex < plotData.size(); curveIndex++ )
    {
        auto curve = new RimPlotCurve();

        curve->setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID );
        curve->setLineThickness( m_curveThickness() );

        // Use the incoming color for all curves, and cycle the symbols
        curve->setColor( RiaColorTools::fromQColorTo3f( color ) );
        auto symbols      = curveSymbols();
        auto customSymbol = symbols[curveIndex % symbols.size()];
        curve->setSymbol( customSymbol );
        curve->setSymbolSize( m_curveSymbolSize() );

        QString curveName;
        if ( curveNameContent.defaultName ) curveName = plotData.curveTitle( curveIndex );

        auto familyValue = ( curveIndex < valueSelection.familyValues.size() ) ? valueSelection.familyValues[curveIndex] : 0.0;

        using pvt = RimVfpDefines::ProductionVariableType;

        if ( m_familyVariable() == pvt::THP )
        {
            curveName += formatCurveNamePart( pvt::THP, familyValue, valueSelection.thpValue, "THP" );
        }
        if ( m_familyVariable() == pvt::GAS_LIQUID_RATIO )
        {
            curveName += formatCurveNamePart( pvt::GAS_LIQUID_RATIO, familyValue, valueSelection.gasLiquidRatioValue, "GLR" );
        }
        if ( m_familyVariable() == pvt::WATER_CUT )
        {
            curveName += formatCurveNamePart( pvt::WATER_CUT, familyValue, valueSelection.waterCutValue, "WC" );
        }
        if ( m_familyVariable() == pvt::ARTIFICIAL_LIFT_QUANTITY )
        {
            curveName += formatCurveNamePart( pvt::ARTIFICIAL_LIFT_QUANTITY, familyValue, valueSelection.artificialLiftQuantityValue, "Lift" );
        }
        if ( m_familyVariable() == pvt::FLOW_RATE )
        {
            curveName += formatCurveNamePart( pvt::FLOW_RATE, familyValue, valueSelection.flowRateValue, "Rate" );
        }

        curve->setCustomName( curveName.trimmed() );
        curve->setParentPlotNoReplot( plotWidget );
        if ( curve->plotCurve() )
        {
            bool useLogarithmicScale = false;
            curve->plotCurve()->setSamplesFromXValuesAndYValues( plotData.xData( curveIndex ), plotData.yData( curveIndex ), useLogarithmicScale );
        }
        curve->updateCurveAppearance();
        curve->appearanceChanged.connect( this, &RimCustomVfpPlot::curveAppearanceChanged );

        m_plotCurves.push_back( curve );

        PlotCurveData plotCurveData;
        plotCurveData.curveName                   = plotData.curveTitle( curveIndex );
        plotCurveData.tableNumber                 = tableNumber;
        plotCurveData.flowRateValue               = valueSelection.flowRateValue;
        plotCurveData.thpValue                    = valueSelection.thpValue;
        plotCurveData.artificialLiftQuantityValue = valueSelection.artificialLiftQuantityValue;
        plotCurveData.waterCutValue               = valueSelection.waterCutValue;
        plotCurveData.gasLiquidRatioValue         = valueSelection.gasLiquidRatioValue;

        m_plotCurveMetaData.emplace_back( plotCurveData );
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

    if ( m_curveValueOptions() == RimVfpDefines::CurveOptionValuesType::UNION_OF_SELECTED_TABLES )
    {
        std::vector<RimVfpTable*> tables = m_comparisonTables.ptrReferencedObjectsByType();
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
std::vector<RimVfpDefines::ProductionVariableType> RimCustomVfpPlot::nonFamilyProductionVariables() const
{
    std::vector<RimVfpDefines::ProductionVariableType> variables;

    auto allVariables = { RimVfpDefines::ProductionVariableType::FLOW_RATE,
                          RimVfpDefines::ProductionVariableType::THP,
                          RimVfpDefines::ProductionVariableType::ARTIFICIAL_LIFT_QUANTITY,
                          RimVfpDefines::ProductionVariableType::WATER_CUT,
                          RimVfpDefines::ProductionVariableType::GAS_LIQUID_RATIO };

    for ( const auto& variable : allVariables )
    {
        if ( variable != m_familyVariable() )
        {
            variables.push_back( variable );
        }
    }
    return variables;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<VfpValueSelection> RimCustomVfpPlot::computeValueSelectionCombinations() const
{
    auto populateValueSelection = []( VfpValueSelection& selection, RimVfpDefines::ProductionVariableType variableType, double value )
    {
        switch ( variableType )
        {
            case RimVfpDefines::ProductionVariableType::FLOW_RATE:
                selection.flowRateValue = value;
                break;
            case RimVfpDefines::ProductionVariableType::THP:
                selection.thpValue = value;
                break;
            case RimVfpDefines::ProductionVariableType::ARTIFICIAL_LIFT_QUANTITY:
                selection.artificialLiftQuantityValue = value;
                break;
            case RimVfpDefines::ProductionVariableType::WATER_CUT:
                selection.waterCutValue = value;
                break;
            case RimVfpDefines::ProductionVariableType::GAS_LIQUID_RATIO:
                selection.gasLiquidRatioValue = value;
                break;
        }
    };

    auto availableVariables = nonFamilyProductionVariables();

    std::vector<std::vector<double>> variableVectors;
    for ( auto variableType : availableVariables )
    {
        variableVectors.push_back( valuesForProductionType( variableType ) );
    }

    std::vector<std::vector<double>> allCombinations;
    std::vector<double>              currentCombination( variableVectors.size() );
    generateCombinations( variableVectors, currentCombination, allCombinations, 0 );

    std::vector<VfpValueSelection> valueSelections;
    for ( const auto& combination : allCombinations )
    {
        VfpValueSelection selection;
        for ( size_t i = 0; i < availableVariables.size(); ++i )
        {
            populateValueSelection( selection, availableVariables[i], combination[i] );
        }

        valueSelections.push_back( selection );
    }

    return valueSelections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiuPlotCurveSymbol::PointSymbolEnum> RimCustomVfpPlot::curveSymbols()
{
    return {
        RiuPlotCurveSymbol::SYMBOL_ELLIPSE,
        RiuPlotCurveSymbol::SYMBOL_CROSS,
        RiuPlotCurveSymbol::SYMBOL_DIAMOND,
        RiuPlotCurveSymbol::SYMBOL_XCROSS,
        RiuPlotCurveSymbol::SYMBOL_LEFT_TRIANGLE,
        RiuPlotCurveSymbol::SYMBOL_STAR1,
        RiuPlotCurveSymbol::SYMBOL_RIGHT_TRIANGLE,
    };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::ColorTable RimCustomVfpPlot::curveColors() const
{
    if ( m_colorLegend->colorArray().size() == 0 )
    {
        return RiaColorTables::summaryCurveDefaultPaletteColors().inverted();
    }

    caf::ColorTable colors( m_colorLegend->colorArray() );
    return colors;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::legendColorsChanged( const caf::SignalEmitter* emitter )
{
    onLoadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::setColorItemCategoryHidden()
{
    if ( m_colorLegend )
    {
        for ( auto item : m_colorLegend->colorLegendItems() )
        {
            item->setCategoryFieldsHidden( true );
        }
    }
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

    {
        auto group = uiOrdering.addNewGroup( "Configuration" );
        group->add( &m_curveMatchingType );
        group->add( &m_curveValueOptions );
        group->add( &m_interpolatedVariable );
        if ( m_tableType == RimVfpDefines::TableType::PRODUCTION )
        {
            group->add( &m_primaryVariable );
            group->add( &m_familyVariable );
        }
    }

    {
        auto group = uiOrdering.addNewGroup( "Table Details" );
        group->add( &m_tableType );
        group->add( &m_tableNumber );
        group->add( &m_referenceDepth );
        group->add( &m_flowingPhase );

        if ( m_tableType == RimVfpDefines::TableType::PRODUCTION )
        {
            group->add( &m_flowingWaterFraction );
            group->add( &m_flowingGasFraction );
        }
    }

    {
        auto group = uiOrdering.addNewGroup( "Comparison Tables" );
        group->setCollapsedByDefault();
        group->add( &m_comparisonTables );
    }

    if ( m_tableType == RimVfpDefines::TableType::PRODUCTION )
    {
        auto selectionDetailsGroup = uiOrdering.addNewGroup( "Selection Details" );
        selectionDetailsGroup->setCollapsedByDefault();

        selectionDetailsGroup->add( &m_flowRate );
        m_flowRate.uiCapability()->setUiHidden( m_primaryVariable() == RimVfpDefines::ProductionVariableType::FLOW_RATE );

        selectionDetailsGroup->add( &m_thp );
        m_thp.uiCapability()->setUiHidden( m_primaryVariable() == RimVfpDefines::ProductionVariableType::THP );

        selectionDetailsGroup->add( &m_waterCut );
        m_waterCut.uiCapability()->setUiHidden( m_primaryVariable() == RimVfpDefines::ProductionVariableType::WATER_CUT );

        selectionDetailsGroup->add( &m_gasLiquidRatio );
        m_gasLiquidRatio.uiCapability()->setUiHidden( m_primaryVariable() == RimVfpDefines::ProductionVariableType::GAS_LIQUID_RATIO );

        selectionDetailsGroup->add( &m_artificialLiftQuantity );
        auto options                    = calculateValueOptions( &m_artificialLiftQuantity );
        bool hideArtificialLiftQuantity = ( m_primaryVariable() == RimVfpDefines::ProductionVariableType::ARTIFICIAL_LIFT_QUANTITY ) ||
                                          ( options.size() < 2 );
        m_artificialLiftQuantity.uiCapability()->setUiHidden( hideArtificialLiftQuantity );
    }

    {
        auto group = uiOrdering.addNewGroup( "Appearance" );
        group->add( &m_curveThickness );
        group->add( &m_curveSymbolSize );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimCustomVfpPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options = RimPlot::calculateValueOptions( fieldNeedingOptions );

    if ( fieldNeedingOptions == &m_flowRate )
    {
        calculateTableValueOptions( RimVfpDefines::ProductionVariableType::FLOW_RATE, options );
    }

    else if ( fieldNeedingOptions == &m_thp )
    {
        calculateTableValueOptions( RimVfpDefines::ProductionVariableType::THP, options );
    }

    else if ( fieldNeedingOptions == &m_artificialLiftQuantity )
    {
        calculateTableValueOptions( RimVfpDefines::ProductionVariableType::ARTIFICIAL_LIFT_QUANTITY, options );
    }

    else if ( fieldNeedingOptions == &m_waterCut )
    {
        calculateTableValueOptions( RimVfpDefines::ProductionVariableType::WATER_CUT, options );
    }

    else if ( fieldNeedingOptions == &m_gasLiquidRatio )
    {
        calculateTableValueOptions( RimVfpDefines::ProductionVariableType::GAS_LIQUID_RATIO, options );
    }

    else if ( fieldNeedingOptions == &m_comparisonTables )
    {
        RimVfpDataCollection* vfpDataCollection = RimVfpDataCollection::instance();
        for ( auto table : vfpDataCollection->vfpTableData() )
        {
            // Exclude main table data object
            if ( table == m_mainDataSource ) continue;

            if ( table->tableType() != m_tableType() ) continue;

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

    else if ( fieldNeedingOptions == &m_curveThickness )
    {
        for ( size_t i = 0; i < 10; i++ )
        {
            options.push_back( caf::PdmOptionItemInfo( QString::number( i + 1 ), QVariant::fromValue( i + 1 ) ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( auto attrib = dynamic_cast<caf::PdmUiTreeSelectionEditorAttribute*>( attribute ) )
    {
        attrib->showTextFilter = false;
    }

    if ( field == &m_mainDataSource )
    {
        RiuTools::enableUpDownArrowsForComboBox( attribute );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimCustomVfpPlot::valuesForProductionType( RimVfpDefines::ProductionVariableType variableType ) const
{
    switch ( variableType )
    {
        case RimVfpDefines::ProductionVariableType::FLOW_RATE:
            return m_flowRate();
        case RimVfpDefines::ProductionVariableType::THP:
            return m_thp();
        case RimVfpDefines::ProductionVariableType::ARTIFICIAL_LIFT_QUANTITY:
            return m_artificialLiftQuantity();
        case RimVfpDefines::ProductionVariableType::WATER_CUT:
            return m_waterCut();
        case RimVfpDefines::ProductionVariableType::GAS_LIQUID_RATIO:
            return m_gasLiquidRatio();
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCustomVfpPlot::calculateTableValueOptions( RimVfpDefines::ProductionVariableType variableType, QList<caf::PdmOptionItemInfo>& options )
{
    auto values = availableValues( variableType );

    for ( double value : values )
    {
        options.push_back(
            caf::PdmOptionItemInfo( QString( "%1 %2" ).arg( convertToDisplayUnit( value, variableType ) ).arg( getDisplayUnit( variableType ) ),
                                    value ) );
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
        initializeSelection();
        zoomAll();
    }

    if ( changedField == &m_comparisonTables || changedField == &m_curveMatchingType || changedField == &m_curveValueOptions ||
         changedField == &m_primaryVariable || changedField == &m_familyVariable )
    {
        initializeSelection();
    }

    loadDataAndUpdate();
    updateLayout();
    updateConnectedEditors();
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
std::vector<double> RimCustomVfpPlot::familyValuesForTable( RimVfpTable* table ) const
{
    if ( !table || !m_mainDataSource || !m_mainDataSource->dataSource() || !m_mainDataSource->dataSource()->vfpTables() ) return {};

    std::vector<double> mainTableFamilyValues = valuesForProductionType( m_familyVariable() );

    if ( m_curveMatchingType() == RimVfpDefines::CurveMatchingType::EXACT )
    {
        return mainTableFamilyValues;
    }

    if ( m_curveMatchingType() == RimVfpDefines::CurveMatchingType::CLOSEST_MATCH_FAMILY )
    {
        auto valuesToMatch = table->dataSource()->vfpTables()->getProductionTableData( table->tableNumber(), m_familyVariable() );
        auto indices       = RigVfpTables::uniqueClosestIndices( mainTableFamilyValues, valuesToMatch );

        std::vector<double> values;
        for ( const auto& i : indices )
        {
            values.push_back( valuesToMatch[i] );
        }

        return values;
    }

    return {};
}
