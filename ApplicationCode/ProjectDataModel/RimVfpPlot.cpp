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

#include "RimEclipseResultCase.h"
#include "RimFlowDiagSolution.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimVfpTableExtractor.h"

#include "RigEclipseCaseData.h"
#include "RigTofWellDistributionCalculator.h"

#include "RiaColorTables.h"
#include "RiaColorTools.h"

#include "RiuQwtPlotWidget.h"
#include "qwt_legend.h"
#include "qwt_legend_label.h"
#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_symbol.h"

#include <QGridLayout>
#include <QTextBrowser>
#include <QWidget>

#include <array>

#include "cvfDebugTimer.h"
#include "cvfTrace.h"

#include "cafPdmUiComboBoxEditor.h"

#include <QFileInfo>

//==================================================================================================
//
//
//
//==================================================================================================

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
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpPlot::RimVfpPlot()
{
    // TODO: add icon
    CAF_PDM_InitObject( "VFP Plot", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_filePath, "FilePath", "File Path", "", "", "" );

    caf::AppEnum<RimVfpPlot::TableType> defaultTableType = RimVfpPlot::TableType::INJECTION;
    CAF_PDM_InitField( &m_tableType, "TableType", defaultTableType, "Table Type", "", "", "" );
    m_tableType.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitField( &m_tableNumber, "TableNumber", -1, "Table Number", "", "", "" );
    m_tableNumber.uiCapability()->setUiReadOnly( true );

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
    cvf::Trace::show( "RimVfpPlot::setAutoScaleXEnabled()" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::setAutoScaleYEnabled( bool /*enabled*/ )
{
    cvf::Trace::show( "RimVfpPlot::setAutoScaleYEnabled()" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::updateAxes()
{
    cvf::Trace::show( "RimVfpPlot::updateAxes()" );
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
    cvf::Trace::show( "RimVfpPlot::updateZoomInQwt()" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::updateZoomFromQwt()
{
    cvf::Trace::show( "RimVfpPlot::updateZoomFromQwt()" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimVfpPlot::asciiDataForPlotExport() const
{
    cvf::Trace::show( "RimVfpPlot::asciiDataForPlotExport()" );
    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::reattachAllCurves()
{
    cvf::Trace::show( "RimVfpPlot::reattachAllCurves()" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::detachAllCurves()
{
    cvf::Trace::show( "RimVfpPlot::detachAllCurves()" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObject* RimVfpPlot::findPdmObjectFromQwtCurve( const QwtPlotCurve* /*curve*/ ) const
{
    cvf::Trace::show( "RimVfpPlot::findPdmObjectFromQwtCurve()" );
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::onAxisSelected( int /*axis*/, bool /*toggle*/ )
{
    cvf::Trace::show( "RimVfpPlot::onAxisSelected()" );
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
    cvf::Trace::show( "RimVfpPlot::viewWidget()" );
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QImage RimVfpPlot::snapshotWindowContent()
{
    cvf::Trace::show( "RimVfpPlot::snapshotWindowContent()" );

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
    cvf::Trace::show( "RimVfpPlot::zoomAll()" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::doRemoveFromCollection()
{
    cvf::Trace::show( "RimVfpPlot::doRemoveFromCollection()" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWidget* RimVfpPlot::doCreatePlotViewWidget( QWidget* mainWindowParent )
{
    cvf::Trace::show( "RimVfpPlot::createViewWidget()" );

    // It seems we risk being called multiple times
    if ( m_plotWidget )
    {
        return m_plotWidget;
    }

    m_plotWidget = new RiuQwtPlotWidget( this, mainWindowParent );

    //    m_plotWidget->setAutoReplot( false );

    updateLegend();
    onLoadDataAndUpdate();

    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlot::deleteViewWidget()
{
    cvf::Trace::show( "RimVfpPlot::deleteViewWidget()" );

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
    cvf::Trace::show( "RimVfpPlot::onLoadDataAndUpdate()" );
    cvf::DebugTimer tim( "RimVfpPlot::onLoadDataAndUpdate()" );

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
        // Try to read the file as an prod table first (most common)
        const std::vector<Opm::VFPProdTable> tables =
            RimVfpTableExtractor::extractVfpProductionTables( filePath.toStdString() );
        if ( !tables.empty() )
        {
            // populateVariabelWidgets( tables[0] );
            m_prodTable.reset( new Opm::VFPProdTable( tables[0] ) );
            m_tableType   = RimVfpPlot::TableType::PRODUCTION;
            m_tableNumber = tables[0].getTableNum();
            populatePlotWidgetWithCurveData( m_plotWidget, tables[0], m_primaryVariable(), m_familyVariable() );
        }
        else
        {
            const std::vector<Opm::VFPInjTable> tables =
                RimVfpTableExtractor::extractVfpInjectionTables( filePath.toStdString() );
            if ( !tables.empty() )
            {
                m_tableType   = RimVfpPlot::TableType::INJECTION;
                m_tableNumber = tables[0].getTableNum();
                populatePlotWidgetWithCurveData( m_plotWidget, tables[0] );
            }
        }

        QFileInfo fi( filePath );
        QString   wellName = fi.baseName();

        const QString plotTitleStr = QString( "%1 VFP Plot" ).arg( wellName );
        m_plotWidget->setTitle( plotTitleStr );

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
    cvf::Trace::show( "RimVfpPlot::populatePlotWidgetWithCurves()" );

    plotWidget->detachItems( QwtPlotItem::Rtti_PlotCurve );
    plotWidget->setAxisScale( QwtPlot::xBottom, 0, 1 );
    plotWidget->setAxisScale( QwtPlot::yLeft, 0, 1 );
    plotWidget->setAxisAutoScale( QwtPlot::xBottom, true );
    plotWidget->setAxisAutoScale( QwtPlot::yLeft, true );
    plotWidget->setAxisTitleText( QwtPlot::xBottom, "Liquid Flow Rate [sm3/d]" );
    plotWidget->setAxisTitleText( QwtPlot::yLeft, "Bottom Hole Pressure [Bar]" );

    for ( size_t thp = 0; thp < table.getTHPAxis().size(); thp++ )
    {
        // Just create some dummy values for now
        size_t              numValues = table.getFloAxis().size();
        std::vector<double> xVals     = table.getFloAxis();
        std::vector<double> yVals( numValues, 0.0 );
        for ( size_t y = 0; y < numValues; y++ )
        {
            // Convert from Pascal to Bar
            yVals[y] = RiaEclipseUnitTools::pascalToBar( table( thp, y ) );
        }

        QColor qtClr = RiaColorTables::wellLogPlotPaletteColors().cycledQColor( thp );

        QwtPlotCurve* curve = new QwtPlotCurve;

        // Convert from Pascal to Bar
        curve->setTitle( QString( "THP: %1 Bar" ).arg( RiaEclipseUnitTools::pascalToBar( table.getTHPAxis()[thp] ) ) );
        QwtSymbol* symbol = new QwtSymbol( QwtSymbol::Ellipse, QBrush( qtClr ), QPen( Qt::red, 2 ), QSize( 8, 8 ) );
        curve->setSymbol( symbol );

        curve->setSamples( xVals.data(), yVals.data(), numValues );
        curve->attach( plotWidget );
        curve->show();
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
    cvf::Trace::show( "RimVfpPlot::populatePlotWidgetWithCurves()" );

    plotWidget->detachItems( QwtPlotItem::Rtti_PlotCurve );
    plotWidget->setAxisScale( QwtPlot::xBottom, 0, 1 );
    plotWidget->setAxisScale( QwtPlot::yLeft, 0, 1 );
    plotWidget->setAxisAutoScale( QwtPlot::xBottom, true );
    plotWidget->setAxisAutoScale( QwtPlot::yLeft, true );

    std::cout << "Datum depth: " << table.getDatumDepth() << std::endl;
    std::cout << "Table number: " << table.getTableNum() << std::endl;
    std::cout << "Flow type: " << static_cast<int>( table.getFloType() ) << std::endl;
    std::cout << "Flo axis: " << table.getFloAxis().size() << std::endl;
    std::cout << "THP axis: " << table.getTHPAxis().size() << std::endl;
    std::cout << "WFR axis: " << table.getWFRAxis().size() << std::endl;
    std::cout << "GFR axis: " << table.getGFRAxis().size() << std::endl;
    std::cout << "ALQ axis: " << table.getALQAxis().size() << std::endl;

    QString xAxisTitle = QString( "%1 [%2]" )
                             .arg( caf::AppEnum<RimVfpPlot::ProductionVariableType>::uiText( primaryVariable ),
                                   getDisplayUnit( primaryVariable ) );
    plotWidget->setAxisTitleText( QwtPlot::xBottom, xAxisTitle );
    QString yAxisTitle = QString( "%1 [%2]" )
                             .arg( caf::AppEnum<RimVfpPlot::InterpolatedVariableType>::uiText( m_interpolatedVariable() ),
                                   getDisplayUnit( RimVfpPlot::ProductionVariableType::THP ) );
    plotWidget->setAxisTitleText( QwtPlot::yLeft, yAxisTitle );

    std::vector<double> primaryAxisValues    = getProductionTableData( table, primaryVariable );
    std::vector<double> familyVariableValues = getProductionTableData( table, familyVariable );
    std::vector<double> thpValues            = getProductionTableData( table, RimVfpPlot::ProductionVariableType::THP );

    for ( size_t familyIdx = 0; familyIdx < familyVariableValues.size(); familyIdx++ )
    {
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

            // Convert from Pascal to Bar
            yVals[y] = convertToDisplayUnit( yVals[y], RimVfpPlot::ProductionVariableType::THP );
        }

        double  familyValue = convertToDisplayUnit( familyVariableValues[familyIdx], familyVariable );
        QString familyUnit  = getDisplayUnit( familyVariable );
        QString familyTitle = QString( "%1: %2 %3" )
                                  .arg( caf::AppEnum<RimVfpPlot::ProductionVariableType>::uiText( familyVariable ) )
                                  .arg( familyValue )
                                  .arg( familyUnit );

        QColor        qtClr = RiaColorTables::wellLogPlotPaletteColors().cycledQColor( familyIdx );
        QwtPlotCurve* curve = createPlotCurve( familyTitle, qtClr );

        for ( size_t i = 0; i < primaryAxisValues.size(); i++ )
            primaryAxisValues[i] = convertToDisplayUnit( primaryAxisValues[i], primaryVariable );

        curve->setSamples( primaryAxisValues.data(), yVals.data(), numValues );
        curve->attach( plotWidget );
        curve->show();
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

    return value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimVfpPlot::getDisplayUnit( RimVfpPlot::ProductionVariableType variableType )

{
    if ( variableType == RimVfpPlot::ProductionVariableType::THP )
        return "Bar";
    else if ( variableType == RimVfpPlot::ProductionVariableType::LIQUID_FLOW_RATE )
        return "sm3/d";
    else if ( variableType == RimVfpPlot::ProductionVariableType::WATER_CUT ||
              variableType == RimVfpPlot::ProductionVariableType::GAS_LIQUID_RATIO )
        return "sm3/sm3";
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

        if ( m_tableType == RimVfpPlot::TableType::PRODUCTION )
        {
            uiOrdering.add( &m_interpolatedVariable );
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
