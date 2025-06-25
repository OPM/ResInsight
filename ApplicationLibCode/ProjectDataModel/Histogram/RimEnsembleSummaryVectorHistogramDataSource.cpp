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

#include "RimEnsembleSummaryVectorHistogramDataSource.h"

#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaQDateTimeTools.h"
#include "RiaResultNames.h"

#include "RifSummaryReaderInterface.h"

#include "Histogram/RimHistogramPlot.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"

#include "RigStatisticsMath.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiLineEditor.h"

CAF_PDM_XML_SOURCE_INIT( RimEnsembleSummaryVectorHistogramDataSource, "EnsembleSummaryVectorHistogramDataSource" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleSummaryVectorHistogramDataSource::RimEnsembleSummaryVectorHistogramDataSource()
{
    CAF_PDM_InitObject( "Ensemble Summary Vector Histogram Data Source", );

    CAF_PDM_InitFieldNoDefault( &m_ensemble, "Ensemble", "Ensemble" );

    CAF_PDM_InitFieldNoDefault( &m_summaryAddressUiField, "SelectedVariableDisplayVar", "Vector" );
    m_summaryAddressUiField.xmlCapability()->disableIO();
    m_summaryAddressUiField.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_summaryAddress, "SummaryAddress", "Summary Address" );
    m_summaryAddress.uiCapability()->setUiTreeChildrenHidden( true );
    m_summaryAddress = new RimSummaryAddress;

    CAF_PDM_InitFieldNoDefault( &m_timeStep, "TimeStep", "Time Step" );
    m_timeStep.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_numBins, "NumBins", 15, "Number of Bins" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleSummaryVectorHistogramDataSource::~RimEnsembleSummaryVectorHistogramDataSource()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimEnsembleSummaryVectorHistogramDataSource::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
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
    else if ( fieldNeedingOptions == &m_summaryAddressUiField )
    {
        appendOptionItemsForSummaryAddresses( &options, m_ensemble );
    }
    else if ( fieldNeedingOptions == &m_timeStep )
    {
        if ( m_ensemble )
        {
            std::set<time_t>       allTimeSteps = m_ensemble->ensembleTimeSteps();
            std::vector<QDateTime> allDateTimes;
            for ( time_t timeStep : allTimeSteps )
            {
                QDateTime dateTime = RiaQDateTimeTools::fromTime_t( timeStep );
                options.push_back( caf::PdmOptionItemInfo( formatDateTime( dateTime ), dateTime ) );
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSummaryVectorHistogramDataSource::appendOptionItemsForSummaryAddresses( QList<caf::PdmOptionItemInfo>* options,
                                                                                        RimSummaryEnsemble*            summaryCaseGroup )
{
    if ( !summaryCaseGroup ) return;

    auto allSummaryCases = summaryCaseGroup->allSummaryCases();

    auto addressesForEnsemble = summaryCaseGroup->ensembleSummaryAddresses();

    for ( const auto& addr : addressesForEnsemble )
    {
        std::string name = addr.uiText();
        QString     s    = QString::fromStdString( name );
        options->push_back( caf::PdmOptionItemInfo( s, QVariant::fromValue( addr ) ) );
    }

    options->push_front( caf::PdmOptionItemInfo( RiaResultNames::undefinedResultName(), QVariant::fromValue( RifEclipseSummaryAddress() ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSummaryVectorHistogramDataSource::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    m_summaryAddressUiField = m_summaryAddress->address();

    uiOrdering.add( &m_ensemble );
    uiOrdering.add( &m_summaryAddressUiField );
    uiOrdering.add( &m_timeStep );
    uiOrdering.add( &m_numBins );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSummaryVectorHistogramDataSource::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                                    const QVariant&            oldValue,
                                                                    const QVariant&            newValue )
{
    if ( changedField == &m_ensemble )
    {
        updateConnectedEditors();
    }
    else if ( changedField == &m_summaryAddressUiField )
    {
        m_summaryAddress->setAddress( m_summaryAddressUiField() );
    }

    dataSourceChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimEnsembleSummaryVectorHistogramDataSource::unitNameY() const
{
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimEnsembleSummaryVectorHistogramDataSource::unitNameX() const
{
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimEnsembleSummaryVectorHistogramDataSource::extractValuesFromEnsemble() const
{
    std::vector<double> values;

    time_t selectedTimestep = m_timeStep().toSecsSinceEpoch();

    auto timeDiff = []( time_t lhs, time_t rhs ) -> time_t
    {
        if ( lhs >= rhs )
        {
            return lhs - rhs;
        }
        return rhs - lhs;
    };

    if ( m_summaryAddress && m_ensemble )
    {
        for ( size_t caseIdx = 0u; caseIdx < m_ensemble->allSummaryCases().size(); ++caseIdx )
        {
            auto summaryCase = m_ensemble->allSummaryCases()[caseIdx];

            RifSummaryReaderInterface* reader = summaryCase->summaryReader();
            if ( !reader ) continue;

            double summaryValue        = std::numeric_limits<double>::infinity();
            time_t closestTimeStep     = 0;
            auto [isOk, summaryValues] = reader->values( m_summaryAddressUiField );
            if ( isOk )
            {
                const std::vector<time_t>& timeSteps = reader->timeSteps( m_summaryAddressUiField );
                for ( size_t i = 0; i < timeSteps.size(); ++i )
                {
                    if ( timeDiff( timeSteps[i], selectedTimestep ) < timeDiff( selectedTimestep, closestTimeStep ) )
                    {
                        summaryValue    = summaryValues[i];
                        closestTimeStep = timeSteps[i];
                    }
                }
            }

            if ( summaryValue != std::numeric_limits<double>::infinity() ) values.push_back( summaryValue );
        }
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimEnsembleSummaryVectorHistogramDataSource::valuesX( RimHistogramPlot::GraphType graphType ) const
{
    std::vector<double> values = extractValuesFromEnsemble();
    if ( values.empty() ) return {};

    auto [min_it, max_it] = std::minmax_element( values.begin(), values.end() );

    double min = *min_it;
    double max = *max_it;
    return computeHistogramBins( min, max, m_numBins, graphType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimEnsembleSummaryVectorHistogramDataSource::valuesY( RimHistogramPlot::GraphType     graphType,
                                                                          RimHistogramPlot::FrequencyType frequencyType ) const
{
    std::vector<double> values = extractValuesFromEnsemble();
    if ( values.empty() ) return {};

    auto [min_it, max_it] = std::minmax_element( values.begin(), values.end() );

    double min = *min_it;
    double max = *max_it;

    std::vector<size_t>    histogram;
    RigHistogramCalculator histCalc( min, max, m_numBins, &histogram );
    histCalc.addData( values );

    double p10 = histCalc.calculatePercentil( 0.1, RigStatisticsMath::PercentileStyle::REGULAR );
    double p50 = histCalc.calculatePercentil( 0.5, RigStatisticsMath::PercentileStyle::REGULAR );
    double p90 = histCalc.calculatePercentil( 0.9, RigStatisticsMath::PercentileStyle::REGULAR );

    RiaLogging::info( QString( "%1: P10=%2 Mean=%3 P90=%4" ).arg( QString::fromStdString( name() ) ).arg( p10 ).arg( p50 ).arg( p90 ) );

    return computeHistogramFrequencies( histogram, graphType, frequencyType );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimEnsembleSummaryVectorHistogramDataSource::name() const
{
    std::string name = "";
    if ( m_ensemble ) name = m_ensemble->name().toStdString();
    if ( !m_summaryAddress->address().uiText().empty() ) name += ", " + m_summaryAddress->address().uiText();
    if ( m_timeStep().isValid() ) name += ", " + formatDateTime( m_timeStep() ).toStdString();

    return name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSummaryVectorHistogramDataSource::setEnsemble( RimSummaryEnsemble* ensemble )
{
    m_ensemble = ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSummaryVectorHistogramDataSource::setSummaryAddress( RifEclipseSummaryAddress& summaryAddress )
{
    m_summaryAddressUiField = summaryAddress;
    m_summaryAddress->setAddress( m_summaryAddressUiField );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSummaryVectorHistogramDataSource::setTimeStep( QDateTime& timeStep )
{
    m_timeStep = timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEnsembleSummaryVectorHistogramDataSource::formatDateTime( const QDateTime& dateTime ) const
{
    QString dateFormatString = RiaQDateTimeTools::dateFormatString( RiaPreferences::current()->dateFormat(),
                                                                    RiaDefines::DateFormatComponents::DATE_FORMAT_YEAR_MONTH_DAY );
    return RiaQDateTimeTools::toStringUsingApplicationLocale( dateTime, dateFormatString );
}
