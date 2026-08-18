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
    const QString& isoDateOrInterval = it->second[stepIndex];
    if ( isoDateOrInterval.isEmpty() ) return false;

    prefetchFromTimeStep( result, it->second, stepIndex );

    return fetchAndDecode( result, isoDateOrInterval, values );
}

//--------------------------------------------------------------------------------------------------
/// A displayed dynamic property is read one time step at a time, and each read is a blocking round trip to
/// Sumo. Fetch a window of the following time steps in one concurrent batch instead, so the reads that follow
/// are served from the connector's blob cache. The window bounds both the requests in flight and the data
/// pulled in for time steps that may never be displayed.
//--------------------------------------------------------------------------------------------------
void RifReaderSumoGridProperty::prefetchFromTimeStep( const QString& propertyName, const std::vector<QString>& timestamps, size_t stepIndex )
{
    if ( !m_connector ) return;

    const size_t batchSize = RiaSumoDefines::gridPropertyPrefetchBatchSize();

    std::vector<QString> batch;
    for ( size_t i = stepIndex; i < timestamps.size() && batch.size() < batchSize; i++ )
    {
        // Skip the time steps this property has no data for, they are never downloaded.
        if ( !timestamps[i].isEmpty() ) batch.push_back( timestamps[i] );
    }

    m_connector->grid().prefetchPropertyData( SumoCaseId( m_caseId ), m_ensembleName, m_gridName, m_realization, propertyName, batch );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderSumoGridProperty::fetchAndDecode( const QString& propertyName, const QString& isoDateOrInterval, std::vector<double>* values )
{
    if ( !m_connector || !m_caseData || !values ) return false;

    QByteArray contents =
        m_connector->grid().propertyData( SumoCaseId( m_caseId ), m_ensembleName, m_gridName, m_realization, propertyName, isoDateOrInterval );

    RiaLogging::debug( std::format( "Sumo grid property '{}' (time '{}'): downloaded {} bytes.",
                                    propertyName.toStdString(),
                                    isoDateOrInterval.toStdString(),
                                    contents.size() ) );

    if ( contents.isEmpty() ) return false;

    std::string        buffer = contents.toStdString();
    std::istringstream stream( buffer, std::ios::binary );

    return RifRoffFileTools::propertyValuesFromStream( stream, m_caseData, propertyName, values );
}
