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

#include "RimCorrelationMatrixPlot.h"

#include "RiaApplication.h"
#include "RiaColorTools.h"
#include "RiaPreferences.h"
#include "RiaStatisticsTools.h"
#include "RiuPlotMainWindowTools.h"
#include "RiuQwtLinearScaleEngine.h"
#include "RiuQwtPlotTools.h"
#include "RiuQwtPlotWidget.h"

#include "RifSummaryReaderInterface.h"

#include "RimDerivedSummaryCase.h"
#include "RimEnsembleCurveSet.h"
#include "RimPlotAxisProperties.h"
#include "RimPlotAxisPropertiesInterface.h"
#include "RimPlotDataFilterCollection.h"
#include "RimRegularLegendConfig.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryPlotAxisFormatter.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "cvfScalarMapper.h"

#include "qwt_plot_marker.h"
#include "qwt_scale_draw.h"
#include "qwt_text.h"

#include <algorithm>
#include <limits>
#include <map>
#include <set>

CAF_PDM_SOURCE_INIT( RimCorrelationMatrixPlot, "CorrelationMatrixPlot" );

class TextScaleDraw : public QwtScaleDraw
{
public:
    TextScaleDraw( const std::map<size_t, QString>& tickLabels )
        : m_tickLabels( tickLabels )
    {
    }

    QwtText label( double value ) const override
    {
        size_t intValue = static_cast<size_t>( value + 0.25 );
        auto   it       = m_tickLabels.find( intValue );
        return it != m_tickLabels.end() ? it->second : "";
    }

private:
    std::map<size_t, QString> m_tickLabels;
};

class CorrelationMatrixRowOrColumn
{
public:
    CorrelationMatrixRowOrColumn( const QString&              label,
                                  const std::vector<double>&  values,
                                  const std::vector<QString>& entryLabels )
        : m_label( label )
        , m_values( values )
        , m_entryLabels( entryLabels )
        , m_correlationSum( 0.0 )
    {
        bool anyValid = false;
        for ( auto value : values )
        {
            if ( RiaCurveDataTools::isValidValue( value, false ) )
            {
                m_correlationSum += value * value;
                anyValid = true;
            }
        }
        if ( !anyValid ) m_correlationSum = std::numeric_limits<double>::infinity();
    }

    QString              m_label;
    std::vector<double>  m_values;
    std::vector<QString> m_entryLabels;
    double               m_correlationSum;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationMatrixPlot::RimCorrelationMatrixPlot()
    : RimAbstractCorrelationPlot()
{
    CAF_PDM_InitObject( "Correlation Plot", ":/CorrelationMatrixPlot16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_correlationFactor, "CorrelationFactor", "Correlation Factor", "", "", "" );
    m_correlationFactor.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_showAbsoluteValues, "CorrelationAbsValues", true, "Show Absolute Values", "", "", "" );
    CAF_PDM_InitField( &m_sortByValues, "CorrelationSorting", true, "Sort Matrix by Values", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_legendConfig, "LegendConfig", "", "", "", "" );
    m_legendConfig = new RimRegularLegendConfig();
    m_legendConfig->setAutomaticRanges( -1.0, 1.0, -1.0, 1.0 );

    m_selectMultipleVectors = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationMatrixPlot::~RimCorrelationMatrixPlot()
{
    removeMdiWindowFromMdiArea();

    cleanupBeforeClose();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationMatrixPlot::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                 const QVariant&            oldValue,
                                                 const QVariant&            newValue )
{
    RimAbstractCorrelationPlot::fieldChangedByUi( changedField, oldValue, newValue );
    if ( changedField == &m_correlationFactor || changedField == &m_showAbsoluteValues || changedField == &m_sortByValues )
    {
        this->updateLegend();
        this->loadDataAndUpdate();
        this->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationMatrixPlot::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* correlationGroup = uiOrdering.addNewGroup( "Correlation Factor Settings" );
    correlationGroup->add( &m_correlationFactor );
    correlationGroup->add( &m_showAbsoluteValues );
    correlationGroup->add( &m_sortByValues );

    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroup( "Summary Vector" );
    m_selectedVarsUiField           = selectedVarsText();

    curveDataGroup->add( &m_selectedVarsUiField );
    curveDataGroup->add( &m_pushButtonSelectSummaryAddress, { false, 1, 0 } );
    curveDataGroup->add( &m_timeStep );

    caf::PdmUiGroup* plotGroup = uiOrdering.addNewGroup( "Plot Settings" );
    plotGroup->add( &m_showPlotTitle );
    plotGroup->add( &m_useAutoPlotTitle );
    plotGroup->add( &m_description );
    m_description.uiCapability()->setUiReadOnly( m_useAutoPlotTitle() );
    RimPlot::defineUiOrdering( uiConfigName, *plotGroup );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimCorrelationMatrixPlot::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options =
        RimAbstractCorrelationPlot::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationMatrixPlot::onLoadDataAndUpdate()
{
    updateMdiWindowVisibility();

    m_selectedVarsUiField = selectedVarsText();

    m_analyserOfSelectedCurveDefs = std::unique_ptr<RiaSummaryCurveDefinitionAnalyser>(
        new RiaSummaryCurveDefinitionAnalyser( this->curveDefinitions() ) );

    if ( m_plotWidget )
    {
        m_plotWidget->detachItems( QwtPlotItem::Rtti_PlotBarChart );
        m_plotWidget->detachItems( QwtPlotItem::Rtti_PlotScale );
        m_plotWidget->detachItems( QwtPlotItem::Rtti_PlotItem );

        createMatrix();

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
void RimCorrelationMatrixPlot::childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField )
{
    this->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationMatrixPlot::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering,
                                                     QString                 uiConfigName /*= "" */ )
{
    uiTreeOrdering.add( m_legendConfig() );
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationMatrixPlot::updateAxes()
{
    if ( !m_plotWidget ) return;

    m_plotWidget->setAxisScaleDraw( QwtPlot::yLeft, new TextScaleDraw( m_resultLabels ) );
    m_plotWidget->setAxisScaleEngine( QwtPlot::yLeft, new RiuQwtLinearScaleEngine );
    m_plotWidget->setAxisTitleText( QwtPlot::yLeft, "Result Vector" );
    m_plotWidget->setAxisTitleEnabled( QwtPlot::yLeft, true );
    m_plotWidget->setAxisFontsAndAlignment( QwtPlot::yLeft, 11, 7, false, Qt::AlignCenter );
    m_plotWidget->setAxisLabelsAndTicksEnabled( QwtPlot::yLeft, true, false );
    m_plotWidget->setAxisRange( QwtPlot::yLeft, 0.0, (double)m_resultLabels.size() + 1 );
    m_plotWidget->setMajorAndMinorTickIntervalsAndRange( QwtPlot::yLeft,
                                                         1.0,
                                                         0.0,
                                                         0.5,
                                                         (double)m_resultLabels.size() - 0.5,
                                                         0.0,
                                                         m_resultLabels.size() );

    auto scaleDraw = new TextScaleDraw( m_paramLabels );
    scaleDraw->setLabelRotation( 30.0 );
    m_plotWidget->setAxisScaleDraw( QwtPlot::xBottom, scaleDraw );

    m_plotWidget->setAxisScaleEngine( QwtPlot::xBottom, new RiuQwtLinearScaleEngine );
    m_plotWidget->setAxisTitleText( QwtPlot::xBottom, "Ensemble Parameter" );
    m_plotWidget->setAxisTitleEnabled( QwtPlot::xBottom, true );
    m_plotWidget->setAxisFontsAndAlignment( QwtPlot::xBottom, 11, 6, false, Qt::AlignCenter | Qt::AlignTop );
    m_plotWidget->setAxisLabelsAndTicksEnabled( QwtPlot::xBottom, true, false );
    m_plotWidget->setAxisRange( QwtPlot::xBottom, 0.0, (double)m_paramLabels.size() + 1 );
    // m_plotWidget->setAutoTickIntervalCounts( QwtPlot::xBottom, m_paramLabels.size(), m_paramLabels.size() );
    m_plotWidget->setMajorAndMinorTickIntervalsAndRange( QwtPlot::xBottom,
                                                         1.0,
                                                         0.0,
                                                         0.5,
                                                         (double)m_paramLabels.size() - 0.5,
                                                         0.0,
                                                         (double)m_paramLabels.size() );

    m_plotWidget->setAxisLabelAlignment( QwtPlot::xBottom, Qt::AlignRight );
}

void eraseInvalidEntries( std::vector<CorrelationMatrixRowOrColumn>& matrix )
{
    matrix.erase( std::remove_if( matrix.begin(),
                                  matrix.end(),
                                  []( auto entry ) {
                                      return !RiaCurveDataTools::isValidValue( entry.m_correlationSum, false );
                                  } ),
                  matrix.end() );
}

void sortEntries( std::vector<CorrelationMatrixRowOrColumn>& matrix )
{
    std::sort( matrix.begin(),
               matrix.end(),
               []( const CorrelationMatrixRowOrColumn& lhs, const CorrelationMatrixRowOrColumn& rhs ) {
                   return lhs.m_correlationSum > rhs.m_correlationSum;
               } );
}

std::vector<CorrelationMatrixRowOrColumn> transpose( const std::vector<CorrelationMatrixRowOrColumn>& matrix )
{
    std::vector<CorrelationMatrixRowOrColumn> transposedMatrix;
    for ( size_t rowIdx = 0u; rowIdx < matrix[0].m_values.size(); ++rowIdx )
    {
        QString              label = matrix[0].m_entryLabels[rowIdx];
        std::vector<double>  values;
        std::vector<QString> entryLabels;
        for ( size_t colIdx = 0u; colIdx < matrix.size(); ++colIdx )
        {
            values.push_back( matrix[colIdx].m_values[rowIdx] );
            entryLabels.push_back( matrix[colIdx].m_label );
        }
        transposedMatrix.push_back( CorrelationMatrixRowOrColumn( label, values, entryLabels ) );
    }
    return transposedMatrix;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationMatrixPlot::createMatrix()
{
    time_t selectedTimestep = m_timeStep().toTime_t();

    m_paramLabels.clear();
    m_resultLabels.clear();

    auto curveDefs = curveDefinitions();
    if ( curveDefs.empty() ) return;

    std::vector<CorrelationMatrixRowOrColumn> correlationMatrixColumns;

    for ( EnsembleParameter parameter : ensembleParameters() )
    {
        if ( parameter.isNumeric() && parameter.isValid() )
        {
            bool                 anyValidResults = false;
            std::vector<double>  correlations;
            std::vector<QString> resultLabels;

            for ( auto curveDef : curveDefs )
            {
                double correlation = std::numeric_limits<double>::infinity();

                auto ensemble = curveDef.ensemble();
                auto address  = curveDef.summaryAddress();

                QString resultLabel = curveDef.curveDefinitionText();

                if ( ensemble )
                {
                    std::vector<double> caseValuesAtTimestep;
                    std::vector<double> parameterValues;

                    for ( size_t caseIdx = 0u; caseIdx < ensemble->allSummaryCases().size(); ++caseIdx )
                    {
                        auto                       summaryCase = ensemble->allSummaryCases()[caseIdx];
                        RifSummaryReaderInterface* reader      = summaryCase->summaryReader();
                        if ( reader )
                        {
                            std::vector<double> values;
                            double              closestValue    = std::numeric_limits<double>::infinity();
                            time_t              closestTimeStep = 0;
                            if ( reader->values( address, &values ) )
                            {
                                const std::vector<time_t>& timeSteps = reader->timeSteps( address );
                                for ( size_t i = 0; i < timeSteps.size(); ++i )
                                {
                                    if ( timeDiff( timeSteps[i], selectedTimestep ) <
                                         timeDiff( selectedTimestep, closestTimeStep ) )
                                    {
                                        closestValue    = values[i];
                                        closestTimeStep = timeSteps[i];
                                    }
                                }
                            }
                            if ( closestValue != std::numeric_limits<double>::infinity() )
                            {
                                caseValuesAtTimestep.push_back( closestValue );
                                double paramValue = parameter.values[caseIdx].toDouble();
                                parameterValues.push_back( paramValue );
                            }
                        }
                    }

                    if ( parameterValues.empty() ) continue;

                    if ( m_correlationFactor == CorrelationFactor::PEARSON )
                    {
                        correlation = RiaStatisticsTools::pearsonCorrelation( parameterValues, caseValuesAtTimestep );
                    }
                    else
                    {
                        correlation = RiaStatisticsTools::spearmanCorrelation( parameterValues, caseValuesAtTimestep );
                    }

                    if ( RiaCurveDataTools::isValidValue( correlation, false ) )
                    {
                        if ( m_showAbsoluteValues() ) correlation = std::abs( correlation );
                        anyValidResults = true;
                    }
                }
                correlations.push_back( correlation );
                resultLabels.push_back( resultLabel );
            }
            if ( anyValidResults )
            {
                correlationMatrixColumns.push_back(
                    CorrelationMatrixRowOrColumn( parameter.name, correlations, resultLabels ) );
            }
        }
    }

    eraseInvalidEntries( correlationMatrixColumns );
    if ( m_sortByValues() ) sortEntries( correlationMatrixColumns );

    auto correlationMatrixRows = transpose( correlationMatrixColumns );

    eraseInvalidEntries( correlationMatrixRows );
    if ( m_sortByValues() ) sortEntries( correlationMatrixRows );

    for ( size_t rowIdx = 0u; rowIdx < correlationMatrixRows.size(); ++rowIdx )
    {
        for ( size_t colIdx = 0u; colIdx < correlationMatrixRows[rowIdx].m_values.size(); ++colIdx )
        {
            double        correlation = correlationMatrixRows[rowIdx].m_values[colIdx];
            auto          label       = QString( "%1" ).arg( correlation, 0, 'f', 2 );
            cvf::Color3ub color       = m_legendConfig->scalarMapper()->mapToColor( correlation );
            QColor        qColor( color.r(), color.g(), color.b() );
            QwtPlotItem*  rectangle = RiuQwtPlotTools::createBoxShape( label,
                                                                      (double)colIdx,
                                                                      (double)colIdx + 1.0,
                                                                      (double)rowIdx,
                                                                      (double)rowIdx + 1,
                                                                      qColor );
            QwtText       textLabel( label );
            cvf::Color3f  contrastColor = RiaColorTools::contrastColor( cvf::Color3f( color ) );
            textLabel.setColor( RiaColorTools::toQColor( contrastColor ) );
            QFont font = textLabel.font();
            font.setPixelSize( RiaFontCache::pointSizeToPixelSize( 7 ) );
            textLabel.setFont( font );
            QwtPlotMarker* marker = new QwtPlotMarker();
            marker->setLabel( textLabel );
            marker->setXValue( colIdx + 0.5 );
            marker->setYValue( rowIdx + 0.5 );
            rectangle->attach( m_plotWidget );
            marker->attach( m_plotWidget );
            m_paramLabels[colIdx] = correlationMatrixRows[rowIdx].m_entryLabels[colIdx];
        }
        m_resultLabels[rowIdx] = correlationMatrixRows[rowIdx].m_label;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationMatrixPlot::updatePlotTitle()
{
    if ( m_useAutoPlotTitle )
    {
        m_description = QString( "%1 Matrix for Parameters vs Result Vectors" ).arg( m_correlationFactor().uiText() );
    }
    m_plotWidget->setPlotTitle( m_description );
    m_plotWidget->setPlotTitleEnabled( m_showPlotTitle && isMdiWindow() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationMatrixPlot::updateLegend()
{
    if ( m_showAbsoluteValues )
    {
        m_legendConfig->setAutomaticRanges( -1.0, 1.0, -1.0, 1.0 );
    }
    else
    {
        m_legendConfig->setAutomaticRanges( 0.0, 1.0, 0.0, 1.0 );
    }
}
