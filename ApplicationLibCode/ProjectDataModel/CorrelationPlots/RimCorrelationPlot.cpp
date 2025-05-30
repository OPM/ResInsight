/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RimCorrelationPlot.h"

#include "RiaColorTools.h"
#include "RiaPreferences.h"
#include "RiaQDateTimeTools.h"

#include "RiuGroupedBarChartBuilder.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuQwtPlotItem.h"
#include "RiuQwtPlotWidget.h"

#include "RifSummaryReaderInterface.h"

#include "RimDeltaSummaryCase.h"
#include "RimEnsembleCurveSet.h"
#include "RimPlotAxisProperties.h"
#include "RimPlotAxisPropertiesInterface.h"
#include "RimPlotDataFilterCollection.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryPlotAxisFormatter.h"

#include "cafFontTools.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include "qwt_plot.h"
#include "qwt_plot_barchart.h"
#include "qwt_text.h"

#include <limits>
#include <map>
#include <set>

CAF_PDM_SOURCE_INIT( RimCorrelationPlot, "CorrelationPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationPlot::RimCorrelationPlot()
    : RimAbstractCorrelationPlot()
    , tornadoItemSelected( this )
{
    CAF_PDM_InitObject( "Correlation Tornado Plot", ":/CorrelationTornadoPlot16x16.png" );

    CAF_PDM_InitField( &m_showAbsoluteValues, "CorrelationAbsValues", false, "Show Absolute Values" );
    CAF_PDM_InitField( &m_sortByAbsoluteValues, "CorrelationAbsSorting", true, "Sort by Absolute Values" );
    CAF_PDM_InitField( &m_excludeParametersWithoutVariation, "ExcludeParamsWithoutVariation", true, "Exclude Parameters Without Variation" );
    CAF_PDM_InitField( &m_showOnlyTopNCorrelations, "ShowOnlyTopNCorrelations", true, "Show Only Top Correlations" );
    CAF_PDM_InitField( &m_topNFilterCount, "TopNFilterCount", 20, "Number rows/columns" );

    CAF_PDM_InitFieldNoDefault( &m_selectedParametersList, "SelectedParameters", "Select Parameters" );
    m_selectedParametersList.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_selectedParametersList.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    // Color taken from https://webviz-subsurface-example.azurewebsites.net/parameters-vs-rft
    QColor qColor = QColor( "#3173b2" );
    CAF_PDM_InitField( &m_barColor, "BarColor", RiaColorTools::fromQColorTo3f( qColor ), "Bar Color" );

    setLegendsVisible( false );
    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationPlot::~RimCorrelationPlot()
{
    if ( isMdiWindow() ) removeMdiWindowFromMdiArea();

    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimAbstractCorrelationPlot::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_showAbsoluteValues || changedField == &m_sortByAbsoluteValues ||
         changedField == &m_excludeParametersWithoutVariation || changedField == &m_selectedParametersList ||
         changedField == &m_showOnlyTopNCorrelations || changedField == &m_topNFilterCount || changedField == &m_barColor )
    {
        if ( changedField == &m_excludeParametersWithoutVariation )
        {
            selectAllParameters();
        }
        loadDataAndUpdate();
        updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* correlationGroup = uiOrdering.addNewGroup( "Correlation Settings" );
    correlationGroup->add( &m_excludeParametersWithoutVariation );
    correlationGroup->add( &m_selectedParametersList );

    correlationGroup->add( &m_showAbsoluteValues );
    if ( !m_showAbsoluteValues() )
    {
        correlationGroup->add( &m_sortByAbsoluteValues );
    }

    correlationGroup->add( &m_showOnlyTopNCorrelations );
    if ( m_showOnlyTopNCorrelations() )
    {
        correlationGroup->add( &m_topNFilterCount );
    }

    appendDataSourceFields( uiConfigName, uiOrdering );

    caf::PdmUiGroup* plotGroup = uiOrdering.addNewGroup( "Plot Settings" );
    plotGroup->add( &m_showPlotTitle );
    plotGroup->add( &m_useAutoPlotTitle );
    plotGroup->add( &m_description );
    RimPlot::defineUiOrdering( uiConfigName, *plotGroup );
    plotGroup->add( &m_titleFontSize );
    plotGroup->add( &m_axisTitleFontSize );
    plotGroup->add( &m_axisValueFontSize );
    plotGroup->add( &m_barColor );

    m_description.uiCapability()->setUiReadOnly( m_useAutoPlotTitle() );
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimCorrelationPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options = RimAbstractCorrelationPlot::calculateValueOptions( fieldNeedingOptions );

    if ( fieldNeedingOptions == &m_selectedParametersList )
    {
        std::set<RigEnsembleParameter> params = variationSortedEnsembleParameters();
        for ( auto param : params )
        {
            if ( !m_excludeParametersWithoutVariation() || param.variationBin > RigEnsembleParameter::NO_VARIATION )
            {
                options.push_back( caf::PdmOptionItemInfo( param.uiName(), param.name ) );
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();

    m_selectedVarsUiField = selectedVectorNamesText();

    if ( m_plotWidget && m_analyserOfSelectedCurveDefs )
    {
        m_plotWidget->qwtPlot()->detachItems( QwtPlotItem::Rtti_PlotBarChart );
        m_plotWidget->qwtPlot()->detachItems( QwtPlotItem::Rtti_PlotScale );

        RiuGroupedBarChartBuilder chartBuilder;

        chartBuilder.setBarColor( RiaColorTools::toQColor( m_barColor() ) );

        addDataToChartBuilder( chartBuilder );

        chartBuilder.addBarChartToPlot( m_plotWidget->qwtPlot(), Qt::Horizontal, m_showOnlyTopNCorrelations() ? m_topNFilterCount() : -1 );
        chartBuilder.setLabelFontSize( labelFontSize() );

        m_plotWidget->qwtPlot()->insertLegend( nullptr );
        m_plotWidget->updateLegend();

        updateAxes();
        updatePlotTitle();
        m_plotWidget->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::updateAxes()
{
    if ( !m_plotWidget ) return;

    m_plotWidget->setAxisTitleText( RiuPlotAxis::defaultLeft(), "Parameter" );
    m_plotWidget->setAxisTitleEnabled( RiuPlotAxis::defaultLeft(), true );
    m_plotWidget->setAxisFontsAndAlignment( RiuPlotAxis::defaultLeft(), axisTitleFontSize(), axisValueFontSize(), false, Qt::AlignCenter );

    m_plotWidget->setAxisTitleText( RiuPlotAxis::defaultBottom(), "Pearson Correlation Coefficient" );
    m_plotWidget->setAxisTitleEnabled( RiuPlotAxis::defaultBottom(), true );
    m_plotWidget->setAxisFontsAndAlignment( RiuPlotAxis::defaultBottom(), axisTitleFontSize(), axisValueFontSize(), false, Qt::AlignCenter );
    if ( m_showAbsoluteValues )
    {
        m_plotWidget->setAxisTitleText( RiuPlotAxis::defaultBottom(), "Pearson Correlation Coefficient ABS" );
        m_plotWidget->setAxisRange( RiuPlotAxis::defaultBottom(), 0.0, 1.0 );
    }
    else
    {
        m_plotWidget->setAxisTitleText( RiuPlotAxis::defaultBottom(), "Pearson Correlation Coefficient" );
        m_plotWidget->setAxisRange( RiuPlotAxis::defaultBottom(), -1.0, 1.0 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::addDataToChartBuilder( RiuGroupedBarChartBuilder& chartBuilder ) const
{
    time_t selectedTimestep = m_timeStep().toSecsSinceEpoch();

    if ( ensembles().empty() ) return;
    if ( addresses().empty() ) return;

    auto ensemble = *ensembles().begin();
    auto address  = *addresses().begin();

    std::set<RimSummaryCase*> activeCases = filterEnsembleCases( ensemble );

    std::vector<std::pair<RigEnsembleParameter, double>> correlations =
        ensemble->parameterCorrelations( address, selectedTimestep, m_selectedParametersList(), activeCases );

    for ( auto parameterCorrPair : correlations )
    {
        double  value     = m_showAbsoluteValues() ? std::abs( parameterCorrPair.second ) : parameterCorrPair.second;
        double  sortValue = m_sortByAbsoluteValues() ? std::abs( value ) : value;
        QString barText   = QString( "%1 (%2)" ).arg( parameterCorrPair.first.name ).arg( parameterCorrPair.second, 5, 'f', 2 );
        QString majorText = "", medText = "", minText = "", legendText = barText;
        chartBuilder.addBarEntry( majorText, medText, minText, sortValue, legendText, barText, value );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::updatePlotTitle()
{
    if ( m_useAutoPlotTitle && !ensembles().empty() )
    {
        QString vectorName = completeAddressText();

        auto ensemble = *ensembles().begin();
        m_description = QString( "%1, %2 at %3" ).arg( vectorName ).arg( ensemble->name() ).arg( timeStepString() );
    }
    m_plotWidget->setPlotTitle( m_description );
    m_plotWidget->setPlotTitleEnabled( m_showPlotTitle && !isSubPlot() );
    m_plotWidget->setPlotTitleFontSize( titleFontSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::onPlotItemSelected( std::shared_ptr<RiuPlotItem> plotItem, bool toggle, int sampleIndex )
{
    RiuQwtPlotItem* qwtPlotItem = dynamic_cast<RiuQwtPlotItem*>( plotItem.get() );
    if ( !qwtPlotItem ) return;

    QwtPlotBarChart* barChart = dynamic_cast<QwtPlotBarChart*>( qwtPlotItem->qwtPlotItem() );
    if ( barChart && !curveDefinitions().empty() )
    {
        auto curveDef = curveDefinitions().front();
        auto barTitle = barChart->title();
        for ( auto param : ensembleParameters() )
        {
            if ( barTitle.text() == param.name )
            {
                tornadoItemSelected.send( std::make_pair( param.name, curveDef ) );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimCorrelationPlot::asciiDataForPlotExport() const
{
    RiuGroupedBarChartBuilder chartBuilder;
    addDataToChartBuilder( chartBuilder );

    return chartBuilder.plotContentAsText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCorrelationPlot::showAbsoluteValues() const
{
    return m_showAbsoluteValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::setShowAbsoluteValues( bool showAbsoluteValues )
{
    m_showAbsoluteValues = showAbsoluteValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimCorrelationPlot::sortByAbsoluteValues() const
{
    return m_sortByAbsoluteValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::setSortByAbsoluteValues( bool sortByAbsoluteValues )
{
    m_sortByAbsoluteValues = sortByAbsoluteValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::selectAllParameters()
{
    m_selectedParametersList.v().clear();
    std::set<RigEnsembleParameter> params = variationSortedEnsembleParameters();
    for ( auto param : params )
    {
        if ( !m_excludeParametersWithoutVariation() || param.variationBin > RigEnsembleParameter::NO_VARIATION )
        {
            m_selectedParametersList.v().push_back( param.name );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::setShowOnlyTopNCorrelations( bool showOnlyTopNCorrelations )
{
    m_showOnlyTopNCorrelations = showOnlyTopNCorrelations;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::setTopNFilterCount( int filterCount )
{
    m_topNFilterCount = filterCount;
}
