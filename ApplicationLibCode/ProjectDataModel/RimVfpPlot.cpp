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

#include "RimVfpTableExtractor.h"

#include "RiaColorTables.h"
#include "RiaEclipseUnitTools.h"

#include "RiuQwtPlotWidget.h"

#include "cafPdmUiComboBoxEditor.h"

#include "qwt_legend.h"
#include "qwt_legend_label.h"
#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_symbol.h"

#include <QFileInfo>

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

namespace caf
{
template <>
void caf::AppEnum<RimVfpPlot::InterpolatedVariableType>::setUp()
{
    addItem( RimVfpPlot::InterpolatedVariableType::BHP, "BHP", "Bottom Hole Pressure" );
    addItem( RimVfpPlot::InterpolatedVariableType::BHP_THP_DIFF, "BHP_THP_DIFF", "BHP-THP" );
    setDefault( RimVfpPlot::InterpolatedVariableType::BHP );
}

template <>
void caf::AppEnum<RimVfpPlot::TableType>::setUp()
{
    addItem( RimVfpPlot::TableType::INJECTION, "INJECTION", "Injection" );
    addItem( RimVfpPlot::TableType::PRODUCTION, "PRODUCTION", "Production" );
    setDefault( RimVfpPlot::TableType::INJECTION );
}

template <>
void caf::AppEnum<RimVfpPlot::ProductionVariableType>::setUp()
{
    addItem( RimVfpPlot::ProductionVariableType::LIQUID_FLOW_RATE, "LIQUID_FLOW_RATE", "Liquid Flow Rate" );
    addItem( RimVfpPlot::ProductionVariableType::THP, "THP", "THP" );
    addItem( RimVfpPlot::ProductionVariableType::WATER_CUT, "WATER_CUT", "Water Cut" );
    addItem( RimVfpPlot::ProductionVariableType::GAS_LIQUID_RATIO, "GAS_LIQUID_RATIO", "Gas Liquid Ratio" );
    addItem( RimVfpPlot::ProductionVariableType::ARTIFICIAL_LIFT_QUANTITY, "ALQ", "Artificial Lift Quantity" );
    setDefault( RimVfpPlot::ProductionVariableType::LIQUID_FLOW_RATE );
}

template <>
void caf::AppEnum<RimVfpPlot::FlowingPhaseType>::setUp()
{
    addItem( RimVfpPlot::FlowingPhaseType::OIL, "OIL", "Oil" );
    addItem( RimVfpPlot::FlowingPhaseType::GAS, "GAS", "Gas" );
    addItem( RimVfpPlot::FlowingPhaseType::WATER, "WATER", "Water" );
    addItem( RimVfpPlot::FlowingPhaseType::LIQUID, "LIQUID", "Liquid (Oil and Water)" );
    addItem( RimVfpPlot::FlowingPhaseType::INVALID, "INVALID", "Invalid" );
    setDefault( RimVfpPlot::FlowingPhaseType::INVALID );
}

template <>
void caf::AppEnum<RimVfpPlot::FlowingWaterFractionType>::setUp()
{
    addItem( RimVfpPlot::FlowingWaterFractionType::WOR, "WOR", "Water-Oil Ratio" );
    addItem( RimVfpPlot::FlowingWaterFractionType::WCT, "WCT", "Water Cut" );
    addItem( RimVfpPlot::FlowingWaterFractionType::WGR, "WGR", "Water-Gas Ratio" );
    addItem( RimVfpPlot::FlowingWaterFractionType::INVALID, "INVALID", "Invalid" );
    setDefault( RimVfpPlot::FlowingWaterFractionType::INVALID );
}

template <>
void caf::AppEnum<RimVfpPlot::FlowingGasFractionType>::setUp()
{
    addItem( RimVfpPlot::FlowingGasFractionType::GOR, "GOR", "Gas-Oil Ratio" );
    addItem( RimVfpPlot::FlowingGasFractionType::GLR, "GLR", "Gas-Liquid Ratio" );
    addItem( RimVfpPlot::FlowingGasFractionType::OGR, "OGR", "Oil-Gas Ratio" );
    addItem( RimVfpPlot::FlowingGasFractionType::INVALID, "INVALID", "Invalid" );
    setDefault( RimVfpPlot::FlowingGasFractionType::INVALID );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpPlot::RimVfpPlot()
{
    // TODO: add icon
    CAF_PDM_InitObject( "VFP Plot", "", "", "" );

    CAF_PDM_InitField( &m_plotTitle, "PlotTitle", QString( "VFP Plot" ), "Plot Title", "", "", "" );
    m_plotTitle.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_filePath, "FilePath", "File Path", "", "", "" );

    caf::AppEnum<RimVfpPlot::TableType> defaultTableType = RimVfpPlot::TableType::INJECTION;
    CAF_PDM_InitField( &m_tableType, "TableType", defaultTableType, "Table Type", "", "", "" );
    m_tableType.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_tableNumber, "TableNumber", -1, "Table Number", "", "", "" );
    m_tableNumber.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_referenceDepth, "ReferenceDepth", 0.0, "Reference Depth", "", "", "" );
    m_referenceDepth.uiCapability()->setUiReadOnly( true );

    caf::AppEnum<RimVfpPlot::FlowingPhaseType> defaultFlowingPhase = RimVfpPlot::FlowingPhaseType::WATER;
    CAF_PDM_InitField( &m_flowingPhase, "FlowingPhase", defaultFlowingPhase, "Flowing Phase", "", "", "" );
    m_flowingPhase.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_flowingWaterFraction, "FlowingWaterFraction", "Flowing Water Fraction", "", "", "" );
    m_flowingWaterFraction.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_flowingGasFraction, "FlowingGasFraction", "Flowing Gas Fraction", "", "", "" );
    m_flowingGasFraction.uiCapability()->setUiReadOnly( true );

    caf::AppEnum<RimVfpPlot::InterpolatedVariableType> defaultInterpolatedVariable =
        RimVfpPlot::InterpolatedVariableType::BHP;
    CAF_PDM_InitField( &m_interpolatedVariable,
                       "InterpolatedVariable",
                       defaultInterpolatedVariable,
                       "Interpolated Variable",
                       "",
                       "",
                       "" );

    caf::AppEnum<RimVfpPlot::ProductionVariableType> defaultPrimaryVariable =
        RimVfpPlot::ProductionVariableType::LIQUID_FLOW_RATE;
    CAF_PDM_InitField( &m_primaryVariable, "PrimaryVariable", defaultPrimaryVariable, "Primary Variable", "", "", "" );

    caf::AppEnum<RimVfpPlot::ProductionVariableType> defaultFamilyVariable = RimVfpPlot::ProductionVariableType::THP;
    CAF_PDM_InitField( &m_familyVariable, "FamilyVariable", defaultFamilyVariable, "Family Variable", "", "", "" );

    CAF_PDM_InitField( &m_liquidFlowRateIdx, "LiquidFlowRateIdx", 0, "Liquid Flow Rate", "", "", "" );
    m_liquidFlowRateIdx.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_thpIdx, "THPIdx", 0, "THP", "", "", "" );
    m_thpIdx.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_articifialLiftQuantityIdx, "ArtificialLiftQuantityIdx", 0, "Artificial Lift Quantity", "", "", "" );
    m_articifialLiftQuantityIdx.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_waterCutIdx, "WaterCutIdx", 0, "Water Cut", "", "", "" );
    m_waterCutIdx.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_gasLiquidRatioIdx, "GasLiquidRatioIdx", 0, "Gas Liquid Ratio", "", "", "" );
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
        m_plotWidget->insertLegend( legend, QwtPlot::BottomLegend );
    }
    else
    {
        m_plotWidget->insertLegend( nullptr );
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
        if ( m_tableType() == RimVfpPlot::TableType::PRODUCTION )
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

        QString plotTitle =
            generatePlotTitle( wellName, m_tableType(), m_interpolatedVariable(), m_primaryVariable(), m_familyVariable() );

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

    m_plotWidget = new RiuQwtPlotWidget( this, mainWindowParent );

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

    m_plotWidget->detachItems( QwtPlotItem::Rtti_PlotCurve );

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
            m_prodTable.reset( new Opm::VFPProdTable( tables[0] ) );
            m_tableType            = RimVfpPlot::TableType::PRODUCTION;
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
                m_injectionTable.reset( new Opm::VFPInjTable( tables[0] ) );
                m_tableType      = RimVfpPlot::TableType::INJECTION;
                m_tableNumber    = tables[0].getTableNum();
                m_referenceDepth = tables[0].getDatumDepth();
                m_flowingPhase   = getFlowingPhaseType( tables[0] );
                populatePlotWidgetWithCurveData( m_plotWidget, tables[0] );
            }
        }

        updatePlotTitle(
            generatePlotTitle( wellName, m_tableType(), m_interpolatedVariable(), m_primaryVariable(), m_familyVariable() ) );

        m_plotWidget->setAxisTitleEnabled( QwtPlot::xBottom, true );
        m_plotWidget->setAxisTitleEnabled( QwtPlot::yLeft, true );
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
void RimVfpPlot::populatePlotData( const Opm::VFPInjTable&              table,
                                   RimVfpPlot::InterpolatedVariableType interpolatedVariable,
                                   VfpPlotData&                         plotData ) const
{
    QString xAxisTitle =
        QString( "%1 %2" ).arg( caf::AppEnum<RimVfpPlot::ProductionVariableType>::uiText(
                                    RimVfpPlot::ProductionVariableType::LIQUID_FLOW_RATE ),
                                getDisplayUnitWithBracket( RimVfpPlot::ProductionVariableType::LIQUID_FLOW_RATE ) );

    plotData.setXAxisTitle( xAxisTitle );

    QString yAxisTitle =
        QString( "%1 %2" ).arg( caf::AppEnum<RimVfpPlot::InterpolatedVariableType>::uiText( m_interpolatedVariable() ),
                                getDisplayUnitWithBracket( RimVfpPlot::ProductionVariableType::THP ) );
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
            if ( m_interpolatedVariable == RimVfpPlot::InterpolatedVariableType::BHP_THP_DIFF )
            {
                yVals[y] -= thpValues[thp];
            }
        }

        double  value = convertToDisplayUnit( thpValues[thp], RimVfpPlot::ProductionVariableType::THP );
        QString unit  = getDisplayUnit( RimVfpPlot::ProductionVariableType::THP );
        QString title =
            QString( "%1: %2 %3" )
                .arg( caf::AppEnum<RimVfpPlot::ProductionVariableType>::uiText( RimVfpPlot::ProductionVariableType::THP ) )
                .arg( value )
                .arg( unit );

        convertToDisplayUnit( yVals, RimVfpPlot::ProductionVariableType::THP );
        convertToDisplayUnit( xVals, RimVfpPlot::ProductionVariableType::LIQUID_FLOW_RATE );

        plotData.appendCurve( title, xVals, yVals );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::populatePlotWidgetWithCurveData( RiuQwtPlotWidget*                  plotWidget,
                                                  const Opm::VFPProdTable&           table,
                                                  RimVfpPlot::ProductionVariableType primaryVariable,
                                                  RimVfpPlot::ProductionVariableType familyVariable )
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
    plotWidget->detachItems( QwtPlotItem::Rtti_PlotCurve );
    plotWidget->setAxisScale( QwtPlot::xBottom, 0, 1 );
    plotWidget->setAxisScale( QwtPlot::yLeft, 0, 1 );
    plotWidget->setAxisAutoScale( QwtPlot::xBottom, true );
    plotWidget->setAxisAutoScale( QwtPlot::yLeft, true );
    plotWidget->setAxisTitleText( QwtPlot::xBottom, plotData.xAxisTitle() );
    plotWidget->setAxisTitleText( QwtPlot::yLeft, plotData.yAxisTitle() );

    for ( unsigned int idx = 0; idx < plotData.size(); idx++ )
    {
        QColor        qtClr = RiaColorTables::wellLogPlotPaletteColors().cycledQColor( idx );
        QwtPlotCurve* curve = createPlotCurve( plotData.curveTitle( idx ), qtClr );
        curve->setSamples( plotData.xData( idx ).data(), plotData.yData( idx ).data(), plotData.curveSize( idx ) );
        curve->attach( plotWidget );
        curve->show();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::populatePlotData( const Opm::VFPProdTable&             table,
                                   RimVfpPlot::ProductionVariableType   primaryVariable,
                                   RimVfpPlot::ProductionVariableType   familyVariable,
                                   RimVfpPlot::InterpolatedVariableType interpolatedVariable,
                                   VfpPlotData&                         plotData ) const
{
    QString xAxisTitle =
        QString( "%1 %2" ).arg( caf::AppEnum<RimVfpPlot::ProductionVariableType>::uiText( primaryVariable ),
                                getDisplayUnitWithBracket( primaryVariable ) );
    plotData.setXAxisTitle( xAxisTitle );
    QString yAxisTitle =
        QString( "%1 %2" ).arg( caf::AppEnum<RimVfpPlot::InterpolatedVariableType>::uiText( interpolatedVariable ),
                                getDisplayUnitWithBracket( RimVfpPlot::ProductionVariableType::THP ) );
    plotData.setYAxisTitle( yAxisTitle );

    size_t numFamilyValues = getProductionTableData( table, familyVariable ).size();
    for ( size_t familyIdx = 0; familyIdx < numFamilyValues; familyIdx++ )
    {
        std::vector<double> primaryAxisValues    = getProductionTableData( table, primaryVariable );
        std::vector<double> familyVariableValues = getProductionTableData( table, familyVariable );
        std::vector<double> thpValues = getProductionTableData( table, RimVfpPlot::ProductionVariableType::THP );

        size_t              numValues = primaryAxisValues.size();
        std::vector<double> yVals( numValues, 0.0 );

        for ( size_t y = 0; y < numValues; y++ )
        {
            size_t wfr_idx = getVariableIndex( table,
                                               RimVfpPlot::ProductionVariableType::WATER_CUT,
                                               primaryVariable,
                                               y,
                                               familyVariable,
                                               familyIdx );
            size_t gfr_idx = getVariableIndex( table,
                                               RimVfpPlot::ProductionVariableType::GAS_LIQUID_RATIO,
                                               primaryVariable,
                                               y,
                                               familyVariable,
                                               familyIdx );
            size_t alq_idx = getVariableIndex( table,
                                               RimVfpPlot::ProductionVariableType::ARTIFICIAL_LIFT_QUANTITY,
                                               primaryVariable,
                                               y,
                                               familyVariable,
                                               familyIdx );
            size_t flo_idx = getVariableIndex( table,
                                               RimVfpPlot::ProductionVariableType::LIQUID_FLOW_RATE,
                                               primaryVariable,
                                               y,
                                               familyVariable,
                                               familyIdx );
            size_t thp_idx =
                getVariableIndex( table, RimVfpPlot::ProductionVariableType::THP, primaryVariable, y, familyVariable, familyIdx );

            yVals[y] = table( thp_idx, wfr_idx, gfr_idx, alq_idx, flo_idx );
            if ( m_interpolatedVariable == RimVfpPlot::InterpolatedVariableType::BHP_THP_DIFF )
            {
                yVals[y] -= thpValues[thp_idx];
            }
        }

        double  familyValue = convertToDisplayUnit( familyVariableValues[familyIdx], familyVariable );
        QString familyUnit  = getDisplayUnit( familyVariable );
        QString familyTitle = QString( "%1: %2 %3" )
                                  .arg( caf::AppEnum<RimVfpPlot::ProductionVariableType>::uiText( familyVariable ) )
                                  .arg( familyValue )
                                  .arg( familyUnit );

        convertToDisplayUnit( yVals, RimVfpPlot::ProductionVariableType::THP );
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
double RimVfpPlot::convertToDisplayUnit( double value, RimVfpPlot::ProductionVariableType variableType )
{
    if ( variableType == RimVfpPlot::ProductionVariableType::THP )
    {
        return RiaEclipseUnitTools::pascalToBar( value );
    }
    else if ( variableType == RimVfpPlot::ProductionVariableType::LIQUID_FLOW_RATE )
    {
        // Convert to m3/sec to m3/day
        return value * static_cast<double>( 24 * 60 * 60 );
    }

    return value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::convertToDisplayUnit( std::vector<double>& values, RimVfpPlot::ProductionVariableType variableType )
{
    for ( size_t i = 0; i < values.size(); i++ )
        values[i] = convertToDisplayUnit( values[i], variableType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimVfpPlot::getDisplayUnitWithBracket( RimVfpPlot::ProductionVariableType variableType )
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
QString RimVfpPlot::getDisplayUnit( RimVfpPlot::ProductionVariableType variableType )

{
    if ( variableType == RimVfpPlot::ProductionVariableType::THP )
        return "Bar";
    else if ( variableType == RimVfpPlot::ProductionVariableType::LIQUID_FLOW_RATE )
        return "m3/day";
    else if ( variableType == RimVfpPlot::ProductionVariableType::WATER_CUT ||
              variableType == RimVfpPlot::ProductionVariableType::GAS_LIQUID_RATIO )
        return "";
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimVfpPlot::getProductionTableData( const Opm::VFPProdTable&           table,
                                                        RimVfpPlot::ProductionVariableType variableType ) const
{
    std::vector<double> xVals;
    if ( variableType == RimVfpPlot::ProductionVariableType::WATER_CUT )
    {
        xVals = table.getWFRAxis();
    }
    else if ( variableType == RimVfpPlot::ProductionVariableType::GAS_LIQUID_RATIO )
    {
        xVals = table.getGFRAxis();
    }
    else if ( variableType == RimVfpPlot::ProductionVariableType::ARTIFICIAL_LIFT_QUANTITY )
    {
        xVals = table.getALQAxis();
    }
    else if ( variableType == RimVfpPlot::ProductionVariableType::LIQUID_FLOW_RATE )
    {
        xVals = table.getFloAxis();
    }
    else if ( variableType == RimVfpPlot::ProductionVariableType::THP )
    {
        xVals = table.getTHPAxis();
    }

    return xVals;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimVfpPlot::getVariableIndex( const Opm::VFPProdTable&           table,
                                     RimVfpPlot::ProductionVariableType targetVariable,
                                     RimVfpPlot::ProductionVariableType primaryVariable,
                                     size_t                             primaryValue,
                                     RimVfpPlot::ProductionVariableType familyVariable,
                                     size_t                             familyValue ) const
{
    if ( targetVariable == primaryVariable )
        return primaryValue;
    else if ( targetVariable == familyVariable )
        return familyValue;
    else
    {
        if ( targetVariable == RimVfpPlot::ProductionVariableType::WATER_CUT )
        {
            return m_waterCutIdx;
        }
        else if ( targetVariable == RimVfpPlot::ProductionVariableType::GAS_LIQUID_RATIO )
        {
            return m_gasLiquidRatioIdx;
        }
        else if ( targetVariable == RimVfpPlot::ProductionVariableType::ARTIFICIAL_LIFT_QUANTITY )
        {
            return m_articifialLiftQuantityIdx;
        }
        else if ( targetVariable == RimVfpPlot::ProductionVariableType::LIQUID_FLOW_RATE )
        {
            return m_liquidFlowRateIdx;
        }
        else if ( targetVariable == RimVfpPlot::ProductionVariableType::THP )
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

        if ( m_tableType == RimVfpPlot::TableType::PRODUCTION )
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
            setFixedVariableUiEditability( m_liquidFlowRateIdx, RimVfpPlot::ProductionVariableType::LIQUID_FLOW_RATE );
            setFixedVariableUiEditability( m_thpIdx, RimVfpPlot::ProductionVariableType::THP );
            setFixedVariableUiEditability( m_articifialLiftQuantityIdx,
                                           RimVfpPlot::ProductionVariableType::ARTIFICIAL_LIFT_QUANTITY );
            setFixedVariableUiEditability( m_waterCutIdx, RimVfpPlot::ProductionVariableType::WATER_CUT );
            setFixedVariableUiEditability( m_gasLiquidRatioIdx, RimVfpPlot::ProductionVariableType::GAS_LIQUID_RATIO );
        }
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::setFixedVariableUiEditability( caf::PdmField<int>& field, RimVfpPlot::ProductionVariableType variableType )
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
        calculateTableValueOptions( RimVfpPlot::ProductionVariableType::LIQUID_FLOW_RATE, options );
    }

    else if ( fieldNeedingOptions == &m_thpIdx )
    {
        calculateTableValueOptions( RimVfpPlot::ProductionVariableType::THP, options );
    }

    else if ( fieldNeedingOptions == &m_articifialLiftQuantityIdx )
    {
        calculateTableValueOptions( RimVfpPlot::ProductionVariableType::ARTIFICIAL_LIFT_QUANTITY, options );
    }

    else if ( fieldNeedingOptions == &m_waterCutIdx )
    {
        calculateTableValueOptions( RimVfpPlot::ProductionVariableType::WATER_CUT, options );
    }

    else if ( fieldNeedingOptions == &m_gasLiquidRatioIdx )
    {
        calculateTableValueOptions( RimVfpPlot::ProductionVariableType::GAS_LIQUID_RATIO, options );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpPlot::FlowingPhaseType RimVfpPlot::getFlowingPhaseType( const Opm::VFPProdTable& table )
{
    switch ( table.getFloType() )
    {
        case Opm::VFPProdTable::FLO_OIL:
            return RimVfpPlot::FlowingPhaseType::OIL;
        case Opm::VFPProdTable::FLO_GAS:
            return RimVfpPlot::FlowingPhaseType::GAS;
        case Opm::VFPProdTable::FLO_LIQ:
            return RimVfpPlot::FlowingPhaseType::LIQUID;
        default:
            return FlowingPhaseType::INVALID;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpPlot::FlowingPhaseType RimVfpPlot::getFlowingPhaseType( const Opm::VFPInjTable& table )
{
    switch ( table.getFloType() )
    {
        case Opm::VFPInjTable::FLO_OIL:
            return RimVfpPlot::FlowingPhaseType::OIL;
        case Opm::VFPInjTable::FLO_GAS:
            return RimVfpPlot::FlowingPhaseType::GAS;
        case Opm::VFPInjTable::FLO_WAT:
            return RimVfpPlot::FlowingPhaseType::WATER;
        default:
            return RimVfpPlot::FlowingPhaseType::INVALID;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpPlot::FlowingGasFractionType RimVfpPlot::getFlowingGasFractionType( const Opm::VFPProdTable& table )
{
    switch ( table.getGFRType() )
    {
        case Opm::VFPProdTable::GFR_GOR:
            return RimVfpPlot::FlowingGasFractionType::GOR;
        case Opm::VFPProdTable::GFR_GLR:
            return RimVfpPlot::FlowingGasFractionType::GLR;
        case Opm::VFPProdTable::GFR_OGR:
            return RimVfpPlot::FlowingGasFractionType::OGR;
        default:
            return RimVfpPlot::FlowingGasFractionType::INVALID;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpPlot::FlowingWaterFractionType RimVfpPlot::getFlowingWaterFractionType( const Opm::VFPProdTable& table )
{
    switch ( table.getWFRType() )
    {
        case Opm::VFPProdTable::WFR_WOR:
            return RimVfpPlot::FlowingWaterFractionType::WOR;
        case Opm::VFPProdTable::WFR_WCT:
            return RimVfpPlot::FlowingWaterFractionType::WCT;
        case Opm::VFPProdTable::WFR_WGR:
            return RimVfpPlot::FlowingWaterFractionType::WGR;
        default:
            return RimVfpPlot::FlowingWaterFractionType::INVALID;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::calculateTableValueOptions( RimVfpPlot::ProductionVariableType variableType,
                                             QList<caf::PdmOptionItemInfo>&     options )
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
        m_plotWidget->setTitle( plotTitle );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimVfpPlot::generatePlotTitle( const QString&                       wellName,
                                       RimVfpPlot::TableType                tableType,
                                       RimVfpPlot::InterpolatedVariableType interpolatedVariable,
                                       RimVfpPlot::ProductionVariableType   primaryVariable,
                                       RimVfpPlot::ProductionVariableType   familyVariable )
{
    QString tableTypeText            = caf::AppEnum<RimVfpPlot::TableType>::uiText( tableType );
    QString interpolatedVariableText = caf::AppEnum<RimVfpPlot::InterpolatedVariableType>::uiText( interpolatedVariable );
    QString primaryVariableText      = caf::AppEnum<RimVfpPlot::ProductionVariableType>::uiText( primaryVariable );
    QString plotTitleStr =
        QString( "VFP: %1 (%2) - %3 x %4" ).arg( wellName ).arg( tableTypeText ).arg( interpolatedVariableText ).arg( primaryVariableText );

    return plotTitleStr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimVfpPlot::userDescriptionField()
{
    return &m_plotTitle;
}
