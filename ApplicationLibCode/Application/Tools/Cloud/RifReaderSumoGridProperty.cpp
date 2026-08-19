/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RifReaderSumoGridProperty.h"

#include "RiaLogging.h"
#include "RiaSumoConnector.h"
#include "RiaSumoDefines.h"

#include "RifRoffFileTools.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigNestedHybridGridResultTools.h"

#include <algorithm>
#include <sstream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderSumoGridProperty::RifReaderSumoGridProperty( RiaSumoConnector* connector,
                                                      const QString&    caseId,
                                                      const QString&    ensembleName,
                                                      const QString&    gridName,
                                                      int               realization )
    : m_connector( connector )
    , m_caseId( caseId )
    , m_ensembleName( ensembleName )
    , m_gridName( gridName )
    , m_realization( realization )
    , m_caseData( nullptr )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderSumoGridProperty::setStaticProperties( const std::vector<QString>& propertyNames )
{
    m_staticProperties = propertyNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderSumoGridProperty::setDynamicProperties( const std::map<QString, std::vector<QString>>& propertyNameToTimestamps )
{
    m_dynamicTimestamps = propertyNameToTimestamps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderSumoGridProperty::open( const QString& /*fileName*/, RigEclipseCaseData* eclipseCase )
{
    // The grid geometry is loaded elsewhere; only keep the case data for cell count and active cell masking.
    m_caseData = eclipseCase;
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderSumoGridProperty::staticResult( const QString& result, RiaDefines::PorosityModelType matrixOrFracture, std::vector<double>* values )
{
    if ( matrixOrFracture != RiaDefines::PorosityModelType::MATRIX_MODEL ) return false;

    // Only fetch properties this reader owns; other static results (e.g. computed DEPTH) are not on Sumo.
    if ( std::find( m_staticProperties.begin(), m_staticProperties.end(), result ) == m_staticProperties.end() ) return false;

    return fetchAndDecode( result, "", values );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
/// A displayed dynamic property is read one time step at a time, and each read is a blocking round trip to
/// Sumo. Download a batch of the following time steps together with the requested one, and decode them
/// straight into the case results, so the reads that follow find the values already loaded and never reach
/// this reader. Nothing is kept in a downloaded form: the decoded values are the only copy.
//--------------------------------------------------------------------------------------------------
bool RifReaderSumoGridProperty::dynamicResult( const QString&                result,
                                               RiaDefines::PorosityModelType matrixOrFracture,
                                               size_t                        stepIndex,
                                               std::vector<double>*          values )
{
    if ( matrixOrFracture != RiaDefines::PorosityModelType::MATRIX_MODEL ) return false;

    auto it = m_dynamicTimestamps.find( result );
    if ( it == m_dynamicTimestamps.end() || stepIndex >= it->second.size() ) return false;

    // The timestamp list is aligned with the case's common time steps. An empty entry means this property has
    // no data at that time step, so report "no data" instead of fetching another step's values.
    const std::vector<QString>& timestamps        = it->second;
    const QString&              isoDateOrInterval = timestamps[stepIndex];
    if ( isoDateOrInterval.isEmpty() ) return false;

    const auto stepsToFetch = timeStepsToFetch( result, timestamps, stepIndex );

    if ( stepsToFetch.size() > 1 )
    {
        std::vector<QString> isoDatesOrIntervals;
        for ( size_t step : stepsToFetch )
        {
            isoDatesOrIntervals.push_back( timestamps[step] );
        }

        const auto contentsByTimestamp =
            m_connector->grid().propertyDataBatch( SumoCaseId( m_caseId ), m_ensembleName, m_gridName, m_realization, result, isoDatesOrIntervals );

        // Decode the look ahead time steps into the case results. They are retained there, so this reader is
        // not called for them again, which is what makes holding on to the downloaded bytes unnecessary.
        for ( size_t step : stepsToFetch )
        {
            if ( step == stepIndex ) continue;

            auto contents = contentsByTimestamp.find( timestamps[step] );
            if ( contents == contentsByTimestamp.end() ) continue;

            // Never resize the time step vector here: the caller holds a reference into it, see
            // RigCaseCellResultsData::findOrLoadKnownScalarResultForTimeStep.
            auto* slot = resultValueSlot( result, step );
            if ( !slot ) continue;

            if ( decodeInto( contents->second, result, slot ) )
            {
                logTransfer( result, timestamps[step], contents->second.size(), true );
            }
        }

        if ( auto contents = contentsByTimestamp.find( isoDateOrInterval ); contents != contentsByTimestamp.end() )
        {
            logTransfer( result, isoDateOrInterval, contents->second.size(), true );

            return decodeInto( contents->second, result, values );
        }
    }

    return fetchAndDecode( result, isoDateOrInterval, values );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t>
    RifReaderSumoGridProperty::timeStepsToFetch( const QString& propertyName, const std::vector<QString>& timestamps, size_t stepIndex )
{
    std::vector<size_t> steps{ stepIndex };

    if ( !m_connector ) return steps;

    const size_t batchSize    = RiaSumoDefines::gridPropertyPrefetchBatchSize();
    const size_t lowWaterMark = RiaSumoDefines::gridPropertyPrefetchLowWaterMark();

    // The values already in the case results are the look ahead: a time step loaded there is served without
    // reaching this reader. Only refill when it runs low, so the batches stay full instead of trickling in
    // one or two time steps at a time.
    std::vector<size_t> unloadedSteps;
    size_t              loadedAhead = 0;
    for ( size_t step = stepIndex + 1; step < timestamps.size(); step++ )
    {
        if ( timestamps[step].isEmpty() ) continue; // no data at this time step for this property

        auto* slot = resultValueSlot( propertyName, step );
        if ( slot && !slot->empty() )
        {
            loadedAhead++;
            continue;
        }

        unloadedSteps.push_back( step );
    }

    if ( loadedAhead >= lowWaterMark ) return steps;

    for ( size_t step : unloadedSteps )
    {
        if ( steps.size() >= batchSize ) break;

        steps.push_back( step );
    }

    // Near the end of the series there is little or nothing left ahead, and a batch of one degrades to a
    // blocking round trip per time step. Fill the rest of the batch with the time steps just behind,
    // nearest first: those are the ones a view is most likely to be asked for next.
    size_t precedingStep = stepIndex;
    while ( precedingStep > 0 && steps.size() < batchSize )
    {
        precedingStep--;

        if ( timestamps[precedingStep].isEmpty() ) continue;

        auto* slot = resultValueSlot( propertyName, precedingStep );
        if ( slot && !slot->empty() ) continue;

        steps.push_back( precedingStep );
    }

    return steps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double>* RifReaderSumoGridProperty::resultValueSlot( const QString& propertyName, size_t stepIndex )
{
    if ( !m_caseData ) return nullptr;

    auto* cellResults = m_caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( !cellResults ) return nullptr;

    const RigEclipseResultAddress resultAddress( RiaDefines::ResultCatType::DYNAMIC_NATIVE, propertyName );
    if ( !cellResults->hasResultEntry( resultAddress ) ) return nullptr;

    auto* timeStepValues = cellResults->modifiableCellScalarResultTimesteps( resultAddress );
    if ( !timeStepValues || stepIndex >= timeStepValues->size() ) return nullptr;

    return &( ( *timeStepValues )[stepIndex] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderSumoGridProperty::fetchAndDecode( const QString& propertyName, const QString& isoDateOrInterval, std::vector<double>* values )
{
    if ( !m_connector || !m_caseData || !values ) return false;

    QByteArray contents =
        m_connector->grid().propertyData( SumoCaseId( m_caseId ), m_ensembleName, m_gridName, m_realization, propertyName, isoDateOrInterval );

    logTransfer( propertyName, isoDateOrInterval, contents.size(), false );

    return decodeInto( contents, propertyName, values );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderSumoGridProperty::logTransfer( const QString& propertyName, const QString& isoDateOrInterval, size_t byteCount, bool fromBatch ) const
{
    RiaLogging::debug( std::format( "Sumo grid property '{}' realization {} (time '{}') [{}]: {} bytes",
                                    propertyName.toStdString(),
                                    m_realization,
                                    isoDateOrInterval.toStdString(),
                                    fromBatch ? "batch" : "single",
                                    byteCount ) );
}

//--------------------------------------------------------------------------------------------------
/// Decode a downloaded roff property blob into cell values. The same post processing as the on demand
/// path in RigCaseCellResultsData is applied, so a prefetched time step is indistinguishable from one
/// read when it was asked for.
//--------------------------------------------------------------------------------------------------
bool RifReaderSumoGridProperty::decodeInto( const QByteArray& contents, const QString& propertyName, std::vector<double>* values )
{
    if ( contents.isEmpty() || !m_caseData || !values ) return false;

    std::string        buffer = contents.toStdString();
    std::istringstream stream( buffer, std::ios::binary );

    if ( !RifRoffFileTools::propertyValuesFromStream( stream, m_caseData, propertyName, values ) ) return false;

    RigNestedHybridGridResultTools::assignValuesToLgrs( m_caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL ), *values );

    return true;
}
