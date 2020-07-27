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

#include "RiaPreferences.h"
#include "RiaStatisticsTools.h"
#include "RiuGroupedBarChartBuilder.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuQwtPlotWidget.h"

#include "RifSummaryReaderInterface.h"

#include "RimDerivedSummaryCase.h"
#include "RimEnsembleCurveSet.h"
#include "RimPlotAxisProperties.h"
#include "RimPlotAxisPropertiesInterface.h"
#include "RimPlotDataFilterCollection.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryPlotAxisFormatter.h"

#include "cafFontTools.h"
#include "cafPdmUiComboBoxEditor.h"

#include "qwt_plot_barchart.h"

#include <limits>
#include <map>
#include <set>

namespace caf
{
template <>
void caf::AppEnum<RimCorrelationPlot::CorrelationFactor>::setUp()
{
    addItem( RimCorrelationPlot::CorrelationFactor::PEARSON, "PEARSON", "Pearson Correlation Coefficient" );
#ifdef USE_GSL
    addItem( RimCorrelationPlot::CorrelationFactor::SPEARMAN, "SPEARMAN", "Spearman's Rank Correlation Coefficient" );
#endif
    setDefault( RimCorrelationPlot::CorrelationFactor::PEARSON );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimCorrelationPlot, "CorrelationPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationPlot::RimCorrelationPlot()
    : RimAbstractCorrelationPlot()
{
    CAF_PDM_InitObject( "Correlation Tornado Plot", ":/CorrelationTornadoPlot16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_correlationFactor, "CorrelationFactor", "Correlation Factor", "", "", "" );
    m_correlationFactor.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_showAbsoluteValues, "CorrelationAbsValues", false, "Show Absolute Values", "", "", "" );
    CAF_PDM_InitField( &m_sortByAbsoluteValues, "CorrelationAbsSorting", true, "Sort by Absolute Values", "", "", "" );
    CAF_PDM_InitField( &m_excludeParametersWithoutVariation,
                       "ExcludeParamsWithoutVariation",
                       true,
                       "Exclude Parameters Without Variation",
                       "",
                       "",
                       "" );

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
void RimCorrelationPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                           const QVariant&            oldValue,
                                           const QVariant&            newValue )
{
    RimAbstractCorrelationPlot::fieldChangedByUi( changedField, oldValue, newValue );
    if ( changedField == &m_correlationFactor || changedField == &m_showAbsoluteValues ||
         changedField == &m_sortByAbsoluteValues )
    {
        this->loadDataAndUpdate();
        this->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* correlationGroup = uiOrdering.addNewGroup( "Correlation Factor Settings" );
    correlationGroup->add( &m_correlationFactor );
    correlationGroup->add( &m_showAbsoluteValues );
    if ( !m_showAbsoluteValues() )
    {
        correlationGroup->add( &m_sortByAbsoluteValues );
    }
    correlationGroup->add( &m_excludeParametersWithoutVariation );

    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup( "Summary Vector" );

    curveDataGroup->add( &m_selectedVarsUiField );
    curveDataGroup->add( &m_pushButtonSelectSummaryAddress, {false, 1, 0} );
    curveDataGroup->add( &m_timeStep );

    caf::PdmUiGroup* plotGroup = uiOrdering.addNewGroup( "Plot Settings" );
    plotGroup->add( &m_showPlotTitle );
    plotGroup->add( &m_useAutoPlotTitle );
    plotGroup->add( &m_description );
    RimPlot::defineUiOrdering( uiConfigName, *plotGroup );
    plotGroup->add( &m_titleFontSize );
    plotGroup->add( &m_axisTitleFontSize );
    plotGroup->add( &m_axisValueFontSize );

    m_description.uiCapability()->setUiReadOnly( m_useAutoPlotTitle() );
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimCorrelationPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                         bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options =
        RimAbstractCorrelationPlot::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();

    m_analyserOfSelectedCurveDefs = std::unique_ptr<RiaSummaryCurveDefinitionAnalyser>(
        new RiaSummaryCurveDefinitionAnalyser( this->curveDefinitions() ) );

    m_selectedVarsUiField = selectedVarsText();

    if ( m_plotWidget && m_analyserOfSelectedCurveDefs )
    {
        m_plotWidget->detachItems( QwtPlotItem::Rtti_PlotBarChart );
        m_plotWidget->detachItems( QwtPlotItem::Rtti_PlotScale );

        RiuGroupedBarChartBuilder chartBuilder;

        // buildTestPlot( chartBuilder );
        addDataToChartBuilder( chartBuilder );

        chartBuilder.addBarChartToPlot( m_plotWidget, Qt::Horizontal );
        chartBuilder.setLabelFontSize( labelFontSize() );

        m_plotWidget->insertLegend( nullptr );
        m_plotWidget->updateLegend();

        this->updateAxes();
        this->updatePlotTitle();
        m_plotWidget->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::updateAxes()
{
    if ( !m_plotWidget ) return;

    m_plotWidget->setAxisTitleText( QwtPlot::yLeft, "Parameter" );
    m_plotWidget->setAxisTitleEnabled( QwtPlot::yLeft, true );
    m_plotWidget->setAxisFontsAndAlignment( QwtPlot::yLeft, axisTitleFontSize(), axisValueFontSize(), false, Qt::AlignCenter );

    m_plotWidget->setAxisTitleText( QwtPlot::xBottom, "Pearson Correlation Coefficient" );
    m_plotWidget->setAxisTitleEnabled( QwtPlot::xBottom, true );
    m_plotWidget->setAxisFontsAndAlignment( QwtPlot::xBottom, axisTitleFontSize(), axisValueFontSize(), false, Qt::AlignCenter );
    if ( m_showAbsoluteValues )
    {
        m_plotWidget->setAxisTitleText( QwtPlot::xBottom, "Pearson Correlation Coefficient ABS" );
        m_plotWidget->setAxisRange( QwtPlot::xBottom, 0.0, 1.0 );
    }
    else
    {
        m_plotWidget->setAxisTitleText( QwtPlot::xBottom, "Pearson Correlation Coefficient" );
        m_plotWidget->setAxisRange( QwtPlot::xBottom, -1.0, 1.0 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::addDataToChartBuilder( RiuGroupedBarChartBuilder& chartBuilder )
{
    time_t selectedTimestep = m_timeStep().toTime_t();

    if ( ensembles().empty() ) return;
    if ( addresses().empty() ) return;

    std::vector<double>                    caseValuesAtTimestep;
    std::map<QString, std::vector<double>> parameterValues;

    auto ensemble = *ensembles().begin();
    auto address  = *addresses().begin();

    for ( size_t caseIdx = 0u; caseIdx < ensemble->allSummaryCases().size(); ++caseIdx )
    {
        auto summaryCase = ensemble->allSummaryCases()[caseIdx];

        RifSummaryReaderInterface* reader = summaryCase->summaryReader();
        if ( !reader ) continue;

        if ( !summaryCase->caseRealizationParameters() ) continue;

        std::vector<double> values;

        double closestValue    = std::numeric_limits<double>::infinity();
        time_t closestTimeStep = 0;
        if ( reader->values( address, &values ) )
        {
            const std::vector<time_t>& timeSteps = reader->timeSteps( address );
            for ( size_t i = 0; i < timeSteps.size(); ++i )
            {
                if ( timeDiff( timeSteps[i], selectedTimestep ) < timeDiff( selectedTimestep, closestTimeStep ) )
                {
                    closestValue    = values[i];
                    closestTimeStep = timeSteps[i];
                }
            }
        }
        if ( closestValue != std::numeric_limits<double>::infinity() )
        {
            caseValuesAtTimestep.push_back( closestValue );

            for ( auto parameter : ensembleParameters() )
            {
                if ( parameter.isNumeric() && parameter.isValid() &&
                     ( !m_excludeParametersWithoutVariation || parameter.normalizedStdDeviation() > 1.0e-5 ) )
                {
                    double paramValue = parameter.values[caseIdx].toDouble();
                    parameterValues[parameter.name].push_back( paramValue );
                }
            }
        }
    }

    std::vector<std::pair<QString, double>> correlationResults;
    for ( auto parameterValuesPair : parameterValues )
    {
        double correlation = 0.0;
        if ( m_correlationFactor == CorrelationFactor::PEARSON )
        {
            correlation = RiaStatisticsTools::pearsonCorrelation( parameterValuesPair.second, caseValuesAtTimestep );
        }
        else
        {
            correlation = RiaStatisticsTools::spearmanCorrelation( parameterValuesPair.second, caseValuesAtTimestep );
        }
        correlationResults.push_back( std::make_pair( parameterValuesPair.first, correlation ) );
    }

    QString timestepString = m_timeStep().toString( RiaPreferences::current()->dateTimeFormat() );

    for ( auto parameterCorrPair : correlationResults )
    {
        double  value     = m_showAbsoluteValues() ? std::abs( parameterCorrPair.second ) : parameterCorrPair.second;
        double  sortValue = m_sortByAbsoluteValues() ? std::abs( value ) : value;
        QString barText   = parameterCorrPair.first;
        QString majorText = "", medText = "", minText = "", legendText = barText;
        chartBuilder.addBarEntry( majorText, medText, minText, sortValue, legendText, barText, value );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::updatePlotTitle()
{
    if ( m_useAutoPlotTitle )
    {
        m_description = QString( "%1 for %2" ).arg( m_correlationFactor().uiText() ).arg( m_selectedVarsUiField );
    }
    m_plotWidget->setPlotTitle( m_description );
    m_plotWidget->setPlotTitleEnabled( m_showPlotTitle && isMdiWindow() );

    if ( isMdiWindow() )
    {
        m_plotWidget->setPlotTitleFontSize( titleFontSize() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::onPlotItemSelected( QwtPlotItem* plotItem, bool toggle, int sampleIndex )
{
    QwtPlotBarChart* barChart = dynamic_cast<QwtPlotBarChart*>( plotItem );
    if ( barChart && !curveDefinitions().empty() )
    {
        auto curveDef = curveDefinitions().front();
        auto barTitle = barChart->title();
        for ( auto param : ensembleParameters() )
        {
            if ( barTitle.text() == param.name )
            {
                emit tornadoItemSelected( param, curveDef );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationPlot::CorrelationFactor RimCorrelationPlot::correlationFactor() const
{
    return m_correlationFactor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlot::setCorrelationFactor( CorrelationFactor factor )
{
    m_correlationFactor = factor;
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
