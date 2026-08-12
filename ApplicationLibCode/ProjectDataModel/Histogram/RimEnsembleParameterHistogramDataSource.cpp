/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RimEnsembleParameterHistogramDataSource.h"

#include "RiaLogging.h"

#include "Histogram/RimHistogramPlot.h"
#include "RimProject.h"
#include "RimSummaryEnsemble.h"

#include "RigStatisticsMath.h"

#include "cafPdmUiTreeSelectionEditor.h"

CAF_PDM_XML_SOURCE_INIT( RimEnsembleParameterHistogramDataSource, "EnsembleParameterHistogramDataSource" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleParameterHistogramDataSource::RimEnsembleParameterHistogramDataSource()
{
    CAF_PDM_InitObject( "Ensemble Parameter Histogram Data Source", );

    CAF_PDM_InitFieldNoDefault( &m_ensemble, "Ensemble", "Ensemble" );
    CAF_PDM_InitFieldNoDefault( &m_parameter, "Parameter", "Parameter" );
    m_parameter.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_numBins, "NumBins", 15, "Number of Bins" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleParameterHistogramDataSource::~RimEnsembleParameterHistogramDataSource()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimEnsembleParameterHistogramDataSource::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_ensemble )
    {
        RimProject* proj = RimProject::current();

        for ( RimSummaryEnsemble* ensemble : proj->summaryEnsembles() )
        {
            if ( ensemble->isEnsemble() ) options.push_back( caf::PdmOptionItemInfo( ensemble->name(), ensemble ) );
        }
    }
    else if ( fieldNeedingOptions == &m_parameter )
    {
        if ( m_ensemble )
        {
            for ( const RigEnsembleParameter& p : m_ensemble->variationSortedEnsembleParameters() )
            {
                if ( p.isNumeric() )
                {
                    options.push_back( caf::PdmOptionItemInfo( p.name, p.name ) );
                }
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleParameterHistogramDataSource ::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_ensemble );
    uiOrdering.add( &m_parameter );
    appendBinningUiOrdering( uiOrdering, &m_numBins );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleParameterHistogramDataSource::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                                const QVariant&            oldValue,
                                                                const QVariant&            newValue )
{
    RimHistogramDataSource::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_ensemble )
    {
        if ( m_ensemble )
        {
            // Try to find a new parameter if the current parameter is empty or not available
            // when changing ensemble.
            auto parameter = m_ensemble->ensembleParameter( m_parameter );
            if ( !parameter.isNumeric() || !parameter.isValid() )
            {
                // Find first valid numeric parameter
                for ( const RigEnsembleParameter& p : m_ensemble->variationSortedEnsembleParameters() )
                {
                    if ( p.isNumeric() && p.isValid() )
                    {
                        m_parameter = p.name;
                        break;
                    }
                }
            }
        }
    }

    dataSourceChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimEnsembleParameterHistogramDataSource::unitNameY() const
{
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimEnsembleParameterHistogramDataSource::unitNameX() const
{
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramDataSource::HistogramResult RimEnsembleParameterHistogramDataSource::compute( RimHistogramPlot::GraphType     graphType,
                                                                                          RimHistogramPlot::FrequencyType frequencyType,
                                                                                          bool                            cumulative ) const
{
    RimHistogramDataSource::HistogramResult result;

    if ( !m_ensemble ) return result;

    auto parameter = m_ensemble->ensembleParameter( m_parameter );
    if ( !parameter.isNumeric() || !parameter.isValid() ) return result;

    std::vector<double> values;
    for ( const QVariant& v : parameter.values )
    {
        values.push_back( v.toDouble() );
    }

    PosNegAccumulator posNegAccumulator;
    posNegAccumulator.addData( values );

    auto [min, max] = binRange( parameter.minValue, parameter.maxValue, posNegAccumulator.pos );
    if ( min > max || RigStatisticsTools::isInvalidNumber( min ) || RigStatisticsTools::isInvalidNumber( max ) ) return result;

    result.valuesX = computeHistogramBins( min, max, m_numBins, graphType, cumulative, binningMode() );

    std::vector<size_t>    histogram;
    RigHistogramCalculator histCalc( min, max, m_numBins, &histogram, binningMode(), outOfRangeHandling() );
    histCalc.addData( values );

    result.valuesY = computeHistogramFrequencies( histogram, graphType, frequencyType, cumulative );

    double p10, p50, p90, mean;
    RigStatisticsMath::calculateStatisticsCurves( values, &p10, &p50, &p90, &mean, RigStatisticsMath::PercentileStyle::SWITCHED );

    result.p10  = p10;
    result.mean = mean;
    result.p90  = p90;

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimEnsembleParameterHistogramDataSource::name() const
{
    std::string name = "";
    if ( m_ensemble ) name = m_ensemble->name().toStdString();
    if ( !m_parameter().isEmpty() ) name += ", " + m_parameter().toStdString();

    return name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleParameterHistogramDataSource::setEnsembleParameter( const QString& ensembleParameter )
{
    m_parameter = ensembleParameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEnsembleParameterHistogramDataSource::ensembleParameter() const
{
    return m_parameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleParameterHistogramDataSource::setEnsemble( RimSummaryEnsemble* ensemble )
{
    m_ensemble = ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryEnsemble* RimEnsembleParameterHistogramDataSource::ensemble() const
{
    return m_ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleParameterHistogramDataSource::setDefaults()
{
    std::vector<RimSummaryEnsemble*> ensembles = RimProject::current()->summaryEnsembles();
    if ( !ensembles.empty() )
    {
        m_ensemble      = ensembles[0];
        auto parameters = m_ensemble->variationSortedEnsembleParameters();
        // Find first valid numeric parameter
        for ( const RigEnsembleParameter& p : m_ensemble->variationSortedEnsembleParameters() )
        {
            if ( p.isNumeric() && p.isValid() )
            {
                m_parameter = p.name;
                break;
            }
        }
    }
}
