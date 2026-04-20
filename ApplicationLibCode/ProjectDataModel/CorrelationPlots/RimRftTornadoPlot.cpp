/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RimRftTornadoPlot.h"

#include "RiaColorTools.h"
#include "RiaPreferences.h"

#include "RigEnsembleParameter.h"
#include "RigStatisticsTools.h"

#include "RimCorrelationBarChartTools.h"
#include "RimEclipseResultCase.h"
#include "RimParameterRftCrossPlot.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryEnsembleTools.h"

#include "RiuContextMenuLauncher.h"
#include "RiuGroupedBarChartBuilder.h"
#include "RiuPlotItem.h"
#include "RiuQwtPlotWidget.h"

#include "cafPdmUiCheckBoxEditor.h"

#include "qwt_plot.h"
#include "qwt_text.h"

#include <QPaintDevice>

#include <limits>
#include <map>
#include <numeric>

CAF_PDM_SOURCE_INIT( RimRftTornadoPlot, "RftTornadoPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRftTornadoPlot::RimRftTornadoPlot()
{
    CAF_PDM_InitObject( "RFT Tornado Plot", ":/CorrelationTornadoPlot16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_ensemble, "Ensemble", "Ensemble" );
    CAF_PDM_InitField( &m_wellName, "WellName", QString(), "Well Name" );
    CAF_PDM_InitFieldNoDefault( &m_selectedTimeStep, "TimeStep", "Time Step" );
    CAF_PDM_InitFieldNoDefault( &m_eclipseCase, "EclipseCase", "Eclipse Case (MD fallback)" );
    CAF_PDM_InitField( &m_useDepthRange, "UseDepthRange", false, "Filter by Depth Range" );
    CAF_PDM_InitField( &m_depthRangeMin, "DepthRangeMin", 0.0, "Min Depth (MD)" );
    CAF_PDM_InitField( &m_depthRangeMax, "DepthRangeMax", 5000.0, "Max Depth (MD)" );

    CAF_PDM_InitField( &m_showAbsoluteValues, "ShowAbsoluteValues", false, "Show Absolute Values" );
    CAF_PDM_InitField( &m_sortByAbsoluteValues, "SortByAbsoluteValues", true, "Sort by Absolute Values" );
    CAF_PDM_InitField( &m_showOnlyTopNCorrelations, "ShowOnlyTopN", true, "Show Only Top Correlations" );
    CAF_PDM_InitField( &m_topNFilterCount, "TopNFilterCount", 20, "Number of Rows" );

    QColor qColor = QColor( "#3173b2" );
    CAF_PDM_InitField( &m_barColor, "BarColor", RiaColorTools::fromQColorTo3f( qColor ), "Bar Color (Positive)" );
    QColor highlightColor = QColor( "#f5a623" );
    CAF_PDM_InitField( &m_highlightBarColor, "HighlightBarColor", RiaColorTools::fromQColorTo3f( highlightColor ), "Bar Color (Selected)" );

    CAF_PDM_InitField( &m_useAutoPlotTitle, "UseAutoPlotTitle", true, "Auto Title" );
    CAF_PDM_InitField( &m_description, "Description", QString( "RFT Tornado Plot" ), "Title" );

    CAF_PDM_InitFieldNoDefault( &m_labelFontSize, "LabelFontSize", "Bar Label Font Size" );
    CAF_PDM_InitFieldNoDefault( &m_axisTitleFontSize, "AxisTitleFontSize", "Axis Title Font Size" );
    CAF_PDM_InitFieldNoDefault( &m_axisValueFontSize, "AxisValueFontSize", "Axis Value Font Size" );

    m_labelFontSize     = caf::FontTools::RelativeSize::Small;
    m_axisTitleFontSize = caf::FontTools::RelativeSize::Small;
    m_axisValueFontSize = caf::FontTools::RelativeSize::Small;

    m_showPlotLegends = false;
    setLegendsVisible( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimRftTornadoPlot::~RimRftTornadoPlot()
{
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTornadoPlot::setEnsemble( RimSummaryEnsemble* ensemble )
{
    m_ensemble = ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTornadoPlot::setWellName( const QString& wellName )
{
    m_wellName = wellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTornadoPlot::setTimeStep( const QDateTime& timeStep )
{
    m_selectedTimeStep = timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTornadoPlot::setEclipseCase( RimEclipseResultCase* eclipseCase )
{
    m_eclipseCase = eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTornadoPlot::setUseDepthRange( bool useDepthRange )
{
    m_useDepthRange = useDepthRange;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTornadoPlot::setDepthRange( double minMd, double maxMd )
{
    m_depthRangeMin = minMd;
    m_depthRangeMax = maxMd;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTornadoPlot::setSelectedParameter( const QString& paramName )
{
    m_selectedParameter = paramName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTornadoPlot::setParameterSelectedCallback( ParameterSelectedCallback callback )
{
    m_parameterSelectedCallback = std::move( callback );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotWidget* RimRftTornadoPlot::viewer()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget* RimRftTornadoPlot::plotWidget()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTornadoPlot::updateAxes()
{
    if ( !m_plotWidget ) return;

    const int titleSize = caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_axisTitleFontSize() );
    const int valueSize = caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_axisValueFontSize() );

    m_plotWidget->setAxisTitleText( RiuPlotAxis::defaultLeft(), "Parameter" );
    m_plotWidget->setAxisTitleEnabled( RiuPlotAxis::defaultLeft(), true );
    m_plotWidget->setAxisFontsAndAlignment( RiuPlotAxis::defaultLeft(), titleSize, valueSize, false, Qt::AlignCenter );

    if ( m_showAbsoluteValues() )
    {
        m_plotWidget->setAxisTitleText( RiuPlotAxis::defaultBottom(), "Pearson Correlation Coefficient ABS" );
        // Use a small negative margin so bar label text (anchored at x=0) remains visible
        m_plotWidget->setAxisRange( RiuPlotAxis::defaultBottom(), -0.15, 1.0 );
    }
    else
    {
        m_plotWidget->setAxisTitleText( RiuPlotAxis::defaultBottom(), "Pearson Correlation Coefficient" );
        m_plotWidget->setAxisRange( RiuPlotAxis::defaultBottom(), -1.0, 1.0 );
    }
    m_plotWidget->setAxisTitleEnabled( RiuPlotAxis::defaultBottom(), true );
    m_plotWidget->setAxisFontsAndAlignment( RiuPlotAxis::defaultBottom(), titleSize, valueSize, false, Qt::AlignCenter );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimRftTornadoPlot::asciiDataForPlotExport() const
{
    RiuGroupedBarChartBuilder chartBuilder;
    addDataToChartBuilder( chartBuilder );
    return chartBuilder.plotContentAsText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTornadoPlot::detachAllCurves()
{
    if ( m_plotWidget )
    {
        m_plotWidget->qwtPlot()->detachItems( QwtPlotItem::Rtti_PlotBarChart );
        m_plotWidget->qwtPlot()->detachItems( QwtPlotItem::Rtti_PlotScale );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimRftTornadoPlot::description() const
{
    return m_description();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* RimRftTornadoPlot::viewWidget()
{
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTornadoPlot::deleteViewWidget()
{
    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTornadoPlot::doRenderWindowContent( QPaintDevice* paintDevice )
{
    if ( m_plotWidget ) m_plotWidget->render( paintDevice );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotWidget* RimRftTornadoPlot::doCreatePlotViewWidget( QWidget* parent )
{
    if ( !m_plotWidget )
    {
        m_plotWidget = new RiuQwtPlotWidget( this, parent );
        updatePlotTitle();
        new RiuContextMenuLauncher( m_plotWidget, { "RicShowPlotDataFeature" } );
    }
    return m_plotWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTornadoPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();

    if ( m_plotWidget )
    {
        detachAllCurves();

        RiuGroupedBarChartBuilder chartBuilder;
        chartBuilder.setBarColor( RiaColorTools::toQColor( m_barColor() ) );
        m_lastCorrelations  = addDataToChartBuilder( chartBuilder );
        const int labelSize = caf::FontTools::absolutePointSize( RiaPreferences::current()->defaultPlotFontSize(), m_labelFontSize() );
        chartBuilder.setLabelFontSize( labelSize );
        chartBuilder.addBarChartToPlot( m_plotWidget->qwtPlot(), Qt::Horizontal, m_showOnlyTopNCorrelations() ? m_topNFilterCount() : -1 );
        RimCorrelationBarChartTools::highlightSelectedParameterBar( m_plotWidget,
                                                                    m_selectedParameter,
                                                                    RiaColorTools::toQColor( m_barColor() ),
                                                                    RiaColorTools::toQColor( m_highlightBarColor() ) );

        m_plotWidget->qwtPlot()->insertLegend( nullptr );

        updateAxes();
        updatePlotTitle();
        m_plotWidget->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTornadoPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto* settingsGroup = uiOrdering.addNewGroup( "Tornado Settings" );
    settingsGroup->add( &m_showAbsoluteValues );
    if ( !m_showAbsoluteValues() ) settingsGroup->add( &m_sortByAbsoluteValues );
    settingsGroup->add( &m_showOnlyTopNCorrelations );
    if ( m_showOnlyTopNCorrelations() ) settingsGroup->add( &m_topNFilterCount );

    auto* colorGroup = uiOrdering.addNewGroup( "Colors" );
    colorGroup->setCollapsedByDefault();
    colorGroup->add( &m_barColor );
    colorGroup->add( &m_highlightBarColor );

    auto* plotGroup = uiOrdering.addNewGroup( "Plot Settings" );
    plotGroup->setCollapsedByDefault();
    plotGroup->add( &m_useAutoPlotTitle );
    plotGroup->add( &m_description );
    plotGroup->add( &m_labelFontSize );
    plotGroup->add( &m_axisTitleFontSize );
    plotGroup->add( &m_axisValueFontSize );
    m_description.uiCapability()->setUiReadOnly( m_useAutoPlotTitle() );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTornadoPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimPlot::fieldChangedByUi( changedField, oldValue, newValue );
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTornadoPlot::onPlotItemSelected( std::shared_ptr<RiuPlotItem> plotItem, bool /*toggle*/, int /*sampleIndex*/ )
{
    const QString paramName = RimCorrelationBarChartTools::parameterNameFromPlotItem( plotItem );
    if ( !paramName.isEmpty() && m_parameterSelectedCallback ) m_parameterSelectedCallback( paramName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, double> RimRftTornadoPlot::addDataToChartBuilder( RiuGroupedBarChartBuilder& chartBuilder ) const
{
    std::map<QString, double> correlations;

    if ( !m_ensemble() || m_wellName().isEmpty() || !m_selectedTimeStep().isValid() ) return correlations;

    const auto& allCases = m_ensemble->allSummaryCases();

    // Build pressure vector per case (indices match allCases order)
    const std::vector<double> pressurePerCase = RimParameterRftCrossPlot::computeMeanPressurePerCase( m_ensemble(),
                                                                                                      m_wellName(),
                                                                                                      m_selectedTimeStep(),
                                                                                                      m_eclipseCase(),
                                                                                                      m_useDepthRange(),
                                                                                                      m_depthRangeMin(),
                                                                                                      m_depthRangeMax() );

    // For each numeric parameter, compute Pearson correlation against pressurePerCase
    for ( const auto& param : RimSummaryEnsembleTools::alphabeticEnsembleParameters( allCases ) )
    {
        if ( !param.isNumeric() ) continue;
        if ( static_cast<size_t>( param.values.size() ) != allCases.size() ) continue;

        std::vector<double> paramValues;
        std::vector<double> pressureValues;
        paramValues.reserve( allCases.size() );
        pressureValues.reserve( allCases.size() );

        for ( size_t i = 0; i < allCases.size(); ++i )
        {
            if ( std::isinf( pressurePerCase[i] ) ) continue;
            paramValues.push_back( param.values[i].toDouble() );
            pressureValues.push_back( pressurePerCase[i] );
        }

        if ( paramValues.size() < 2 ) continue;

        double pearson = RigStatisticsTools::pearsonCorrelation( paramValues, pressureValues );
        if ( std::isinf( pearson ) || std::isnan( pearson ) ) continue;

        correlations[param.name] = pearson;

        RimCorrelationBarChartTools::addCorrelationBar( chartBuilder, param.name, pearson, m_showAbsoluteValues(), m_sortByAbsoluteValues() );
    }

    return correlations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTornadoPlot::updatePlotTitle()
{
    if ( !m_plotWidget ) return;

    if ( m_useAutoPlotTitle() && m_ensemble() )
    {
        const QString rangeStr = m_useDepthRange() ? QString( " [MD %1 - %2 m]" ).arg( m_depthRangeMin() ).arg( m_depthRangeMax() )
                                                   : QString();
        m_description          = QString( "Parameter Correlation vs RFT Pressure%1, %2" ).arg( rangeStr ).arg( m_ensemble->name() );
    }

    m_plotWidget->setPlotTitle( m_description() );
    m_plotWidget->setPlotTitleEnabled( m_showPlotTitle() );
    m_plotWidget->setPlotTitleFontSize( titleFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimRftTornadoPlot::cleanupBeforeClose()
{
    detachAllCurves();

    if ( m_plotWidget )
    {
        m_plotWidget->setParent( nullptr );
        delete m_plotWidget;
        m_plotWidget = nullptr;
    }
}
