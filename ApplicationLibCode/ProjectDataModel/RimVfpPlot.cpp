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

#include "RimVfpPlot.h"

#include "RiaDefines.h"
#include "RimVfpDefines.h"
#include "RimVfpTableExtractor.h"

#include "RiaColorTables.h"
#include "RiaEclipseUnitTools.h"

#include "RiuContextMenuLauncher.h"
#include "RiuQwtPlotTools.h"
#include "RiuQwtPlotWidget.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiComboBoxEditor.h"

#include "qwt_legend.h"
#include "qwt_legend_label.h"
#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_symbol.h"

#include <QFileInfo>

#include <memory>

//==================================================================================================
//
//
//
//==================================================================================================

class VfpPlotData
{
public:
    void setXAxisTitle( const QString& xAxisTitle ) { m_xAxisTitle = xAxisTitle; }
    void setYAxisTitle( const QString& yAxisTitle ) { m_yAxisTitle = yAxisTitle; }

    const QString& xAxisTitle() const { return m_xAxisTitle; }
    const QString& yAxisTitle() const { return m_yAxisTitle; }

    void appendCurve( const QString& curveTitle, const std::vector<double>& xData, const std::vector<double>& yData )
    {
        m_curveTitles.push_back( curveTitle );
        m_xData.push_back( xData );
        m_yData.push_back( yData );
    }

    const QString& curveTitle( size_t idx ) const { return m_curveTitles[idx]; }

    size_t size() const { return m_xData.size(); }

    size_t curveSize( size_t idx ) const { return m_xData[idx].size(); }

    const std::vector<double>& xData( size_t idx ) const { return m_xData[idx]; }
    const std::vector<double>& yData( size_t idx ) const { return m_yData[idx]; }

private:
    QString                          m_xAxisTitle;
    QString                          m_yAxisTitle;
    std::vector<QString>             m_curveTitles;
    std::vector<std::vector<double>> m_xData;
    std::vector<std::vector<double>> m_yData;
};

CAF_PDM_SOURCE_INIT( RimVfpPlot, "VfpPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpPlot::RimVfpPlot()
{
    // TODO: add icon
    CAF_PDM_InitObject( "VFP Plot", ":/VfpPlot.svg", "", "" );

    CAF_PDM_InitField( &m_plotTitle, "PlotTitle", QString( "VFP Plot" ), "Plot Title" );
    m_plotTitle.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_filePath, "FilePath", "File Path" );

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

    caf::AppEnum<RimVfpDefines::InterpolatedVariableType> defaultInterpolatedVariable =
        RimVfpDefines::InterpolatedVariableType::BHP;
    CAF_PDM_InitField( &m_interpolatedVariable,
                       "InterpolatedVariable",
                       defaultInterpolatedVariable,
                       "Interpolated Variable",
                       "",
                       "",
                       "" );

    caf::AppEnum<RimVfpDefines::ProductionVariableType> defaultPrimaryVariable =
        RimVfpDefines::ProductionVariableType::LIQUID_FLOW_RATE;
    CAF_PDM_InitField( &m_primaryVariable, "PrimaryVariable", defaultPrimaryVariable, "Primary Variable" );

    caf::AppEnum<RimVfpDefines::ProductionVariableType> defaultFamilyVariable = RimVfpDefines::ProductionVariableType::THP;
    CAF_PDM_InitField( &m_familyVariable, "FamilyVariable", defaultFamilyVariable, "Family Variable" );

    CAF_PDM_InitField( &m_liquidFlowRateIdx, "LiquidFlowRateIdx", 0, "Liquid Flow Rate" );
    m_liquidFlowRateIdx.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_thpIdx, "THPIdx", 0, "THP" );
    m_thpIdx.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_articifialLiftQuantityIdx, "ArtificialLiftQuantityIdx", 0, "Artificial Lift Quantity" );
    m_articifialLiftQuantityIdx.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_waterCutIdx, "WaterCutIdx", 0, "Water Cut" );
    m_waterCutIdx.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_gasLiquidRatioIdx, "GasLiquidRatioIdx", 0, "Gas Liquid Ratio" );
    m_gasLiquidRatioIdx.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    m_showWindow      = true;
    m_showPlotLegends = true;

    setAsPlotMdiWindow();

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpPlot::~RimVfpPlot()
{
    removeMdiWindowFromMdiArea();
    deleteViewWidget();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::setFileName( const QString& filename )
{
    m_filePath = filename;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWidget* RimVfpPlot::viewer()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::setAutoScaleXEnabled( bool /*enabled*/ )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::setAutoScaleYEnabled( bool /*enabled*/ )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::updateAxes()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::updateLegend()
{
    if ( !m_plotWidget )
    {
        return;
    }

    // Hide the legend when in multiplot mode, as the legend is handeled by the multi plot grid layout
    bool doShowLegend = false;
    if ( isMdiWindow() )
    {
        doShowLegend = m_showPlotLegends;
    }

    if ( doShowLegend )
    {
        QwtLegend* legend = new QwtLegend( m_plotWidget );
        m_plotWidget->qwtPlot()->insertLegend( legend, QwtPlot::BottomLegend );
    }
    else
    {
        m_plotWidget->qwtPlot()->insertLegend( nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::updateZoomInQwt()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::updateZoomFromQwt()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimVfpPlot::asciiDataForPlotExport() const
{
    QString filePath = m_filePath.v().path();
    if ( !filePath.isEmpty() )
    {
        QFileInfo fi( filePath );
        QString   wellName = fi.baseName();

        VfpPlotData plotData;
        if ( m_tableType() == RimVfpDefines::TableType::PRODUCTION )
        {
            if ( m_prodTable )
            {
                populatePlotData( *m_prodTable, m_primaryVariable(), m_familyVariable(), m_interpolatedVariable(), plotData );
            }
        }
        else
        {
            if ( m_injectionTable )
            {
                populatePlotData( *m_injectionTable.get(), m_interpolatedVariable(), plotData );
            }
        }

        QString plotTitle = generatePlotTitle( wellName,
                                               m_tableNumber(),
                                               m_tableType(),
                                               m_interpolatedVariable(),
                                               m_primaryVariable(),
                                               m_familyVariable() );

        QString dataText;

        if ( plotData.size() > 0 )
        {
            // The curves should have same dimensions
            const size_t curveSize = plotData.curveSize( 0 );

            // Generate the headers for the columns
            // First column is the primary variable
            QString columnTitleLine( plotData.xAxisTitle() );

            // Then one column per "family"
            for ( size_t s = 0; s < plotData.size(); s++ )
            {
                columnTitleLine.append( QString( "\t%1" ).arg( plotData.curveTitle( s ) ) );
            }
            columnTitleLine.append( "\n" );

            dataText.append( columnTitleLine );

            // Add the rows: one row per primary variable value
            for ( size_t idx = 0; idx < curveSize; idx++ )
            {
                QString line;

                // First item on each line is the primary variable
                line.append( QString( "%1" ).arg( plotData.xData( 0 )[idx] ) );

                for ( size_t s = 0; s < plotData.size(); s++ )
                {
                    line.append( QString( "\t%1" ).arg( plotData.yData( s )[idx] ) );
                }
                dataText.append( line );
                dataText.append( "\n" );
            }
        }

        return QString( "%1\n\n%2" ).arg( plotTitle ).arg( dataText );
    }

    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::reattachAllCurves()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::detachAllCurves()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObject* RimVfpPlot::findPdmObjectFromQwtCurve( const QwtPlotCurve* /*curve*/ ) const
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::onAxisSelected( int /*axis*/, bool /*toggle*/ )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimVfpPlot::description() const
{
    return uiName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimVfpPlot::viewWidget()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimVfpPlot::snapshotWindowContent()
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
void RimVfpPlot::zoomAll()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::doRemoveFromCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWidget* RimVfpPlot::doCreatePlotViewWidget( QWidget* mainWindowParent )
{
    // It seems we risk being called multiple times
    if ( m_plotWidget )
    {
        return m_plotWidget;
    }

    {
        auto plotWidget = new RiuQwtPlotWidget( this, mainWindowParent );

        // Remove event filter to disable unwanted highlighting on left click in plot.
        plotWidget->removeEventFilter();

        RiuQwtPlotTools::setCommonPlotBehaviour( plotWidget->qwtPlot() );

        caf::CmdFeatureMenuBuilder menuBuilder;
        menuBuilder << "RicShowPlotDataFeature";
        new RiuContextMenuLauncher( plotWidget, menuBuilder );

        m_plotWidget = plotWidget;
    }

    updateLegend();
    onLoadDataAndUpdate();

    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::deleteViewWidget()
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
void RimVfpPlot::onLoadDataAndUpdate()
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

    m_plotWidget->qwtPlot()->detachItems( QwtPlotItem::Rtti_PlotCurve );

    updateLegend();

    QString filePath = m_filePath.v().path();
    if ( !filePath.isEmpty() )
    {
        QFileInfo fi( filePath );
        QString   wellName = fi.baseName();

        // Try to read the file as an prod table first (most common)
        const std::vector<Opm::VFPProdTable> tables =
            RimVfpTableExtractor::extractVfpProductionTables( filePath.toStdString() );
        if ( !tables.empty() )
        {
            m_prodTable            = std::make_unique<Opm::VFPProdTable>( tables[0] );
            m_tableType            = RimVfpDefines::TableType::PRODUCTION;
            m_tableNumber          = tables[0].getTableNum();
            m_referenceDepth       = tables[0].getDatumDepth();
            m_flowingPhase         = getFlowingPhaseType( tables[0] );
            m_flowingGasFraction   = getFlowingGasFractionType( tables[0] );
            m_flowingWaterFraction = getFlowingWaterFractionType( tables[0] );
            populatePlotWidgetWithCurveData( m_plotWidget, tables[0], m_primaryVariable(), m_familyVariable() );
        }
        else
        {
            const std::vector<Opm::VFPInjTable> tables =
                RimVfpTableExtractor::extractVfpInjectionTables( filePath.toStdString() );
            if ( !tables.empty() )
            {
                m_injectionTable = std::make_unique<Opm::VFPInjTable>( tables[0] );
                m_tableType      = RimVfpDefines::TableType::INJECTION;
                m_tableNumber    = tables[0].getTableNum();
                m_referenceDepth = tables[0].getDatumDepth();
                m_flowingPhase   = getFlowingPhaseType( tables[0] );
                populatePlotWidgetWithCurveData( m_plotWidget, tables[0] );
            }
        }

        updatePlotTitle( generatePlotTitle( wellName,
                                            m_tableNumber(),
                                            m_tableType(),
                                            m_interpolatedVariable(),
                                            m_primaryVariable(),
                                            m_familyVariable() ) );

        m_plotWidget->setAxisTitleEnabled( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM, true );
        m_plotWidget->setAxisTitleEnabled( RiaDefines::PlotAxis::PLOT_AXIS_LEFT, true );
    }

    m_plotWidget->scheduleReplot();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::populatePlotWidgetWithCurveData( RiuQwtPlotWidget* plotWidget, const Opm::VFPInjTable& table )
{
    VfpPlotData plotData;
    populatePlotData( table, m_interpolatedVariable(), plotData );
    populatePlotWidgetWithPlotData( plotWidget, plotData );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::populatePlotData( const Opm::VFPInjTable&                 table,
                                   RimVfpDefines::InterpolatedVariableType interpolatedVariable,
                                   VfpPlotData&                            plotData ) const
{
    QString xAxisTitle =
        QString( "%1 %2" ).arg( caf::AppEnum<RimVfpDefines::ProductionVariableType>::uiText(
                                    RimVfpDefines::ProductionVariableType::LIQUID_FLOW_RATE ),
                                getDisplayUnitWithBracket( RimVfpDefines::ProductionVariableType::LIQUID_FLOW_RATE ) );

    plotData.setXAxisTitle( xAxisTitle );

    QString yAxisTitle =
        QString( "%1 %2" ).arg( caf::AppEnum<RimVfpDefines::InterpolatedVariableType>::uiText( m_interpolatedVariable() ),
                                getDisplayUnitWithBracket( RimVfpDefines::ProductionVariableType::THP ) );
    plotData.setYAxisTitle( yAxisTitle );

    std::vector<double> thpValues = table.getTHPAxis();

    for ( size_t thp = 0; thp < thpValues.size(); thp++ )
    {
        size_t              numValues = table.getFloAxis().size();
        std::vector<double> xVals     = table.getFloAxis();
        std::vector<double> yVals( numValues, 0.0 );
        for ( size_t y = 0; y < numValues; y++ )
        {
            yVals[y] = table( thp, y );
            if ( m_interpolatedVariable == RimVfpDefines::InterpolatedVariableType::BHP_THP_DIFF )
            {
                yVals[y] -= thpValues[thp];
            }
        }

        double  value = convertToDisplayUnit( thpValues[thp], RimVfpDefines::ProductionVariableType::THP );
        QString unit  = getDisplayUnit( RimVfpDefines::ProductionVariableType::THP );
        QString title = QString( "%1: %2 %3" )
                            .arg( caf::AppEnum<RimVfpDefines::ProductionVariableType>::uiText(
                                RimVfpDefines::ProductionVariableType::THP ) )
                            .arg( value )
                            .arg( unit );

        convertToDisplayUnit( yVals, RimVfpDefines::ProductionVariableType::THP );
        convertToDisplayUnit( xVals, RimVfpDefines::ProductionVariableType::LIQUID_FLOW_RATE );

        plotData.appendCurve( title, xVals, yVals );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::populatePlotWidgetWithCurveData( RiuQwtPlotWidget*                     plotWidget,
                                                  const Opm::VFPProdTable&              table,
                                                  RimVfpDefines::ProductionVariableType primaryVariable,
                                                  RimVfpDefines::ProductionVariableType familyVariable )
{
    VfpPlotData plotData;
    populatePlotData( table, primaryVariable, familyVariable, m_interpolatedVariable(), plotData );
    populatePlotWidgetWithPlotData( plotWidget, plotData );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::populatePlotWidgetWithPlotData( RiuQwtPlotWidget* plotWidget, const VfpPlotData& plotData )
{
    plotWidget->qwtPlot()->detachItems( QwtPlotItem::Rtti_PlotCurve );
    plotWidget->setAxisScale( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM, 0, 1 );
    plotWidget->setAxisScale( RiaDefines::PlotAxis::PLOT_AXIS_LEFT, 0, 1 );
    plotWidget->setAxisAutoScale( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM, true );
    plotWidget->setAxisAutoScale( RiaDefines::PlotAxis::PLOT_AXIS_LEFT, true );
    plotWidget->setAxisTitleText( RiaDefines::PlotAxis::PLOT_AXIS_BOTTOM, plotData.xAxisTitle() );
    plotWidget->setAxisTitleText( RiaDefines::PlotAxis::PLOT_AXIS_LEFT, plotData.yAxisTitle() );

    for ( auto idx = 0u; idx < plotData.size(); idx++ )
    {
        QColor        qtClr = RiaColorTables::summaryCurveDefaultPaletteColors().cycledQColor( idx );
        QwtPlotCurve* curve = createPlotCurve( plotData.curveTitle( idx ), qtClr );
        curve->setSamples( plotData.xData( idx ).data(),
                           plotData.yData( idx ).data(),
                           static_cast<int>( plotData.curveSize( idx ) ) );
        curve->attach( plotWidget->qwtPlot() );
        curve->show();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::populatePlotData( const Opm::VFPProdTable&                table,
                                   RimVfpDefines::ProductionVariableType   primaryVariable,
                                   RimVfpDefines::ProductionVariableType   familyVariable,
                                   RimVfpDefines::InterpolatedVariableType interpolatedVariable,
                                   VfpPlotData&                            plotData ) const
{
    QString xAxisTitle =
        QString( "%1 %2" ).arg( caf::AppEnum<RimVfpDefines::ProductionVariableType>::uiText( primaryVariable ),
                                getDisplayUnitWithBracket( primaryVariable ) );
    plotData.setXAxisTitle( xAxisTitle );
    QString yAxisTitle =
        QString( "%1 %2" ).arg( caf::AppEnum<RimVfpDefines::InterpolatedVariableType>::uiText( interpolatedVariable ),
                                getDisplayUnitWithBracket( RimVfpDefines::ProductionVariableType::THP ) );
    plotData.setYAxisTitle( yAxisTitle );

    size_t numFamilyValues = getProductionTableData( table, familyVariable ).size();
    for ( size_t familyIdx = 0; familyIdx < numFamilyValues; familyIdx++ )
    {
        std::vector<double> primaryAxisValues    = getProductionTableData( table, primaryVariable );
        std::vector<double> familyVariableValues = getProductionTableData( table, familyVariable );
        std::vector<double> thpValues = getProductionTableData( table, RimVfpDefines::ProductionVariableType::THP );

        size_t              numValues = primaryAxisValues.size();
        std::vector<double> yVals( numValues, 0.0 );

        for ( size_t y = 0; y < numValues; y++ )
        {
            size_t wfr_idx = getVariableIndex( table,
                                               RimVfpDefines::ProductionVariableType::WATER_CUT,
                                               primaryVariable,
                                               y,
                                               familyVariable,
                                               familyIdx );
            size_t gfr_idx = getVariableIndex( table,
                                               RimVfpDefines::ProductionVariableType::GAS_LIQUID_RATIO,
                                               primaryVariable,
                                               y,
                                               familyVariable,
                                               familyIdx );
            size_t alq_idx = getVariableIndex( table,
                                               RimVfpDefines::ProductionVariableType::ARTIFICIAL_LIFT_QUANTITY,
                                               primaryVariable,
                                               y,
                                               familyVariable,
                                               familyIdx );
            size_t flo_idx = getVariableIndex( table,
                                               RimVfpDefines::ProductionVariableType::LIQUID_FLOW_RATE,
                                               primaryVariable,
                                               y,
                                               familyVariable,
                                               familyIdx );
            size_t thp_idx = getVariableIndex( table,
                                               RimVfpDefines::ProductionVariableType::THP,
                                               primaryVariable,
                                               y,
                                               familyVariable,
                                               familyIdx );

            yVals[y] = table( thp_idx, wfr_idx, gfr_idx, alq_idx, flo_idx );
            if ( m_interpolatedVariable == RimVfpDefines::InterpolatedVariableType::BHP_THP_DIFF )
            {
                yVals[y] -= thpValues[thp_idx];
            }
        }

        double  familyValue = convertToDisplayUnit( familyVariableValues[familyIdx], familyVariable );
        QString familyUnit  = getDisplayUnit( familyVariable );
        QString familyTitle = QString( "%1: %2 %3" )
                                  .arg( caf::AppEnum<RimVfpDefines::ProductionVariableType>::uiText( familyVariable ) )
                                  .arg( familyValue )
                                  .arg( familyUnit );

        convertToDisplayUnit( yVals, RimVfpDefines::ProductionVariableType::THP );
        convertToDisplayUnit( primaryAxisValues, primaryVariable );

        plotData.appendCurve( familyTitle, primaryAxisValues, yVals );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtPlotCurve* RimVfpPlot::createPlotCurve( const QString title, const QColor& color )
{
    QwtPlotCurve* curve = new QwtPlotCurve;
    curve->setTitle( title );
    curve->setPen( QPen( color, 2 ) );
    curve->setLegendAttribute( QwtPlotCurve::LegendShowLine, true );
    curve->setLegendAttribute( QwtPlotCurve::LegendShowSymbol, true );
    curve->setLegendAttribute( QwtPlotCurve::LegendShowBrush, true );
    curve->setRenderHint( QwtPlotItem::RenderAntialiased, true );

    QwtSymbol* symbol = new QwtSymbol( QwtSymbol::Ellipse );
    symbol->setSize( 6 );
    symbol->setColor( color );
    curve->setSymbol( symbol );

    return curve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimVfpPlot::convertToDisplayUnit( double value, RimVfpDefines::ProductionVariableType variableType )
{
    if ( variableType == RimVfpDefines::ProductionVariableType::THP )
    {
        return RiaEclipseUnitTools::pascalToBar( value );
    }
    else if ( variableType == RimVfpDefines::ProductionVariableType::LIQUID_FLOW_RATE )
    {
        // Convert to m3/sec to m3/day
        return value * static_cast<double>( 24 * 60 * 60 );
    }

    return value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::convertToDisplayUnit( std::vector<double>& values, RimVfpDefines::ProductionVariableType variableType )
{
    for ( double& value : values )
        value = convertToDisplayUnit( value, variableType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimVfpPlot::getDisplayUnitWithBracket( RimVfpDefines::ProductionVariableType variableType )
{
    QString unit = getDisplayUnit( variableType );
    if ( !unit.isEmpty() )
        return QString( "[%1]" ).arg( unit );
    else
        return unit;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimVfpPlot::getDisplayUnit( RimVfpDefines::ProductionVariableType variableType )

{
    if ( variableType == RimVfpDefines::ProductionVariableType::THP )
        return "Bar";
    else if ( variableType == RimVfpDefines::ProductionVariableType::LIQUID_FLOW_RATE )
        return "m3/day";
    else if ( variableType == RimVfpDefines::ProductionVariableType::WATER_CUT ||
              variableType == RimVfpDefines::ProductionVariableType::GAS_LIQUID_RATIO )
        return "";
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimVfpPlot::getProductionTableData( const Opm::VFPProdTable&              table,
                                                        RimVfpDefines::ProductionVariableType variableType ) const
{
    std::vector<double> xVals;
    if ( variableType == RimVfpDefines::ProductionVariableType::WATER_CUT )
    {
        xVals = table.getWFRAxis();
    }
    else if ( variableType == RimVfpDefines::ProductionVariableType::GAS_LIQUID_RATIO )
    {
        xVals = table.getGFRAxis();
    }
    else if ( variableType == RimVfpDefines::ProductionVariableType::ARTIFICIAL_LIFT_QUANTITY )
    {
        xVals = table.getALQAxis();
    }
    else if ( variableType == RimVfpDefines::ProductionVariableType::LIQUID_FLOW_RATE )
    {
        xVals = table.getFloAxis();
    }
    else if ( variableType == RimVfpDefines::ProductionVariableType::THP )
    {
        xVals = table.getTHPAxis();
    }

    return xVals;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimVfpPlot::getVariableIndex( const Opm::VFPProdTable&              table,
                                     RimVfpDefines::ProductionVariableType targetVariable,
                                     RimVfpDefines::ProductionVariableType primaryVariable,
                                     size_t                                primaryValue,
                                     RimVfpDefines::ProductionVariableType familyVariable,
                                     size_t                                familyValue ) const
{
    if ( targetVariable == primaryVariable )
        return primaryValue;
    else if ( targetVariable == familyVariable )
        return familyValue;
    else
    {
        if ( targetVariable == RimVfpDefines::ProductionVariableType::WATER_CUT )
        {
            return m_waterCutIdx;
        }
        else if ( targetVariable == RimVfpDefines::ProductionVariableType::GAS_LIQUID_RATIO )
        {
            return m_gasLiquidRatioIdx;
        }
        else if ( targetVariable == RimVfpDefines::ProductionVariableType::ARTIFICIAL_LIFT_QUANTITY )
        {
            return m_articifialLiftQuantityIdx;
        }
        else if ( targetVariable == RimVfpDefines::ProductionVariableType::LIQUID_FLOW_RATE )
        {
            return m_liquidFlowRateIdx;
        }
        else if ( targetVariable == RimVfpDefines::ProductionVariableType::THP )
        {
            return m_thpIdx;
        }

        return getProductionTableData( table, targetVariable ).size() - 1;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_filePath );

    if ( !m_filePath.v().path().isEmpty() )
    {
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
            fixedVariablesGroup->add( &m_liquidFlowRateIdx );
            fixedVariablesGroup->add( &m_thpIdx );
            fixedVariablesGroup->add( &m_articifialLiftQuantityIdx );
            fixedVariablesGroup->add( &m_waterCutIdx );
            fixedVariablesGroup->add( &m_gasLiquidRatioIdx );

            // Disable the choices for variables as primary or family
            setFixedVariableUiEditability( m_liquidFlowRateIdx, RimVfpDefines::ProductionVariableType::LIQUID_FLOW_RATE );
            setFixedVariableUiEditability( m_thpIdx, RimVfpDefines::ProductionVariableType::THP );
            setFixedVariableUiEditability( m_articifialLiftQuantityIdx,
                                           RimVfpDefines::ProductionVariableType::ARTIFICIAL_LIFT_QUANTITY );
            setFixedVariableUiEditability( m_waterCutIdx, RimVfpDefines::ProductionVariableType::WATER_CUT );
            setFixedVariableUiEditability( m_gasLiquidRatioIdx, RimVfpDefines::ProductionVariableType::GAS_LIQUID_RATIO );
        }
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::setFixedVariableUiEditability( caf::PdmField<int>&                   field,
                                                RimVfpDefines::ProductionVariableType variableType )
{
    field.uiCapability()->setUiReadOnly( variableType == m_primaryVariable.v() || variableType == m_familyVariable.v() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimVfpPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                 bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options = RimPlot::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );

    if ( fieldNeedingOptions == &m_liquidFlowRateIdx )
    {
        calculateTableValueOptions( RimVfpDefines::ProductionVariableType::LIQUID_FLOW_RATE, options );
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

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpDefines::FlowingPhaseType RimVfpPlot::getFlowingPhaseType( const Opm::VFPProdTable& table )
{
    switch ( table.getFloType() )
    {
        case Opm::VFPProdTable::FLO_OIL:
            return RimVfpDefines::FlowingPhaseType::OIL;
        case Opm::VFPProdTable::FLO_GAS:
            return RimVfpDefines::FlowingPhaseType::GAS;
        case Opm::VFPProdTable::FLO_LIQ:
            return RimVfpDefines::FlowingPhaseType::LIQUID;
        default:
            return RimVfpDefines::FlowingPhaseType::INVALID;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpDefines::FlowingPhaseType RimVfpPlot::getFlowingPhaseType( const Opm::VFPInjTable& table )
{
    switch ( table.getFloType() )
    {
        case Opm::VFPInjTable::FLO_OIL:
            return RimVfpDefines::FlowingPhaseType::OIL;
        case Opm::VFPInjTable::FLO_GAS:
            return RimVfpDefines::FlowingPhaseType::GAS;
        case Opm::VFPInjTable::FLO_WAT:
            return RimVfpDefines::FlowingPhaseType::WATER;
        default:
            return RimVfpDefines::FlowingPhaseType::INVALID;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpDefines::FlowingGasFractionType RimVfpPlot::getFlowingGasFractionType( const Opm::VFPProdTable& table )
{
    switch ( table.getGFRType() )
    {
        case Opm::VFPProdTable::GFR_GOR:
            return RimVfpDefines::FlowingGasFractionType::GOR;
        case Opm::VFPProdTable::GFR_GLR:
            return RimVfpDefines::FlowingGasFractionType::GLR;
        case Opm::VFPProdTable::GFR_OGR:
            return RimVfpDefines::FlowingGasFractionType::OGR;
        default:
            return RimVfpDefines::FlowingGasFractionType::INVALID;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpDefines::FlowingWaterFractionType RimVfpPlot::getFlowingWaterFractionType( const Opm::VFPProdTable& table )
{
    switch ( table.getWFRType() )
    {
        case Opm::VFPProdTable::WFR_WOR:
            return RimVfpDefines::FlowingWaterFractionType::WOR;
        case Opm::VFPProdTable::WFR_WCT:
            return RimVfpDefines::FlowingWaterFractionType::WCT;
        case Opm::VFPProdTable::WFR_WGR:
            return RimVfpDefines::FlowingWaterFractionType::WGR;
        default:
            return RimVfpDefines::FlowingWaterFractionType::INVALID;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::calculateTableValueOptions( RimVfpDefines::ProductionVariableType variableType,
                                             QList<caf::PdmOptionItemInfo>&        options )
{
    if ( m_prodTable )
    {
        std::vector<double> values = getProductionTableData( *m_prodTable.get(), variableType );

        for ( size_t i = 0; i < values.size(); i++ )
        {
            options.push_back( caf::PdmOptionItemInfo( QString( "%1 %2" )
                                                           .arg( convertToDisplayUnit( values[i], variableType ) )
                                                           .arg( getDisplayUnit( variableType ) ),
                                                       static_cast<int>( i ) ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimPlot::fieldChangedByUi( changedField, oldValue, newValue );
    loadDataAndUpdate();
    updateLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::updatePlotTitle( const QString& plotTitle )
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
QString RimVfpPlot::generatePlotTitle( const QString&                          wellName,
                                       int                                     tableNumber,
                                       RimVfpDefines::TableType                tableType,
                                       RimVfpDefines::InterpolatedVariableType interpolatedVariable,
                                       RimVfpDefines::ProductionVariableType   primaryVariable,
                                       RimVfpDefines::ProductionVariableType   familyVariable )
{
    QString tableTypeText = caf::AppEnum<RimVfpDefines::TableType>::uiText( tableType );
    QString interpolatedVariableText =
        caf::AppEnum<RimVfpDefines::InterpolatedVariableType>::uiText( interpolatedVariable );
    QString primaryVariableText = caf::AppEnum<RimVfpDefines::ProductionVariableType>::uiText( primaryVariable );
    QString plotTitleStr        = QString( "VFP: %1 (%2) #%3 - %4 x %5" )
                               .arg( wellName )
                               .arg( tableTypeText )
                               .arg( tableNumber )
                               .arg( interpolatedVariableText )
                               .arg( primaryVariableText );

    return plotTitleStr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimVfpPlot::userDescriptionField()
{
    return &m_plotTitle;
}
