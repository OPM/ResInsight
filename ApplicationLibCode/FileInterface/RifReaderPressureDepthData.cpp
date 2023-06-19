/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-  Equinor ASA
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
#include "RifReaderPressureDepthData.h"

#include "RiaLogging.h"

#include "RifPressureDepthTextFileReader.h"

#include "cafAssert.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderPressureDepthData::RifReaderPressureDepthData( const QString& filePath )
    : m_filePath( filePath )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RifReaderPressureDepthData::labels( const RifEclipseRftAddress& rftAddress )
{
    std::vector<QString> formationLabels;

    if ( m_pressureDepthDataItems.empty() )
    {
        load();
    }

    for ( const RigPressureDepthData& pressureDepthData : m_pressureDepthDataItems )
    {
        if ( rftAddress.wellName() == pressureDepthData.wellName() && rftAddress.timeStep().date() == pressureDepthData.timeStep().date() )
        {
            formationLabels.push_back(
                QString( "%1 - Pressure: %2" ).arg( pressureDepthData.wellName() ).arg( pressureDepthData.timeStep().toString() ) );
        }
    }

    return formationLabels;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseRftAddress> RifReaderPressureDepthData::eclipseRftAddresses()
{
    if ( m_pressureDepthDataItems.empty() )
    {
        load();
    }

    std::set<RifEclipseRftAddress> allAddresses;
    for ( const RigPressureDepthData& pressureDepthData : m_pressureDepthDataItems )
    {
        const QString&   wellName = pressureDepthData.wellName();
        const QDateTime& dateTime = pressureDepthData.timeStep();

        RifEclipseRftAddress tvdAddress =
            RifEclipseRftAddress::createAddress( wellName, dateTime, RifEclipseRftAddress::RftWellLogChannelType::TVD );
        RifEclipseRftAddress pressureAddress =
            RifEclipseRftAddress::createAddress( wellName, dateTime, RifEclipseRftAddress::RftWellLogChannelType::PRESSURE );
        allAddresses.insert( tvdAddress );
        allAddresses.insert( pressureAddress );
    }

    return allAddresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderPressureDepthData::values( const RifEclipseRftAddress& rftAddress, std::vector<double>* values )
{
    CAF_ASSERT( values );

    if ( m_pressureDepthDataItems.empty() )
    {
        load();
    }

    for ( const RigPressureDepthData& pressureDepthData : m_pressureDepthDataItems )
    {
        if ( rftAddress.wellName() == pressureDepthData.wellName() && rftAddress.timeStep().date() == pressureDepthData.timeStep().date() )
        {
            switch ( rftAddress.wellLogChannel() )
            {
                case RifEclipseRftAddress::RftWellLogChannelType::TVD:
                    *values = pressureDepthData.tvdmsl();
                    break;
                case RifEclipseRftAddress::RftWellLogChannelType::PRESSURE:
                    *values = pressureDepthData.pressure();
                    break;
                default:
                    *values = {};
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderPressureDepthData::load()
{
    auto [pressureDepthDataItems, errorMsg] = RifPressureDepthTextFileReader::readFile( m_filePath );
    if ( !errorMsg.isEmpty() )
    {
        RiaLogging::error( errorMsg );
    }
    else
    {
        m_pressureDepthDataItems = pressureDepthDataItems;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QDateTime> RifReaderPressureDepthData::availableTimeSteps( const QString&                                     wellName,
                                                                    const RifEclipseRftAddress::RftWellLogChannelType& wellLogChannelName )
{
    if ( wellLogChannelName == RifEclipseRftAddress::RftWellLogChannelType::TVD ||
         wellLogChannelName == RifEclipseRftAddress::RftWellLogChannelType::PRESSURE )
    {
        return availableTimeSteps( wellName );
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QDateTime> RifReaderPressureDepthData::availableTimeSteps( const QString& wellName )
{
    if ( m_pressureDepthDataItems.empty() )
    {
        load();
    }

    std::set<QDateTime> timeSteps;
    for ( const RigPressureDepthData& pressureDepthData : m_pressureDepthDataItems )
    {
        if ( wellName == pressureDepthData.wellName() )
        {
            timeSteps.insert( pressureDepthData.timeStep() );
        }
    }
    return timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QDateTime> RifReaderPressureDepthData::availableTimeSteps( const QString& wellName,
                                                                    const std::set<RifEclipseRftAddress::RftWellLogChannelType>& relevantChannels )
{
    if ( relevantChannels.count( RifEclipseRftAddress::RftWellLogChannelType::TVD ) ||
         relevantChannels.count( RifEclipseRftAddress::RftWellLogChannelType::PRESSURE ) )
    {
        return availableTimeSteps( wellName );
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseRftAddress::RftWellLogChannelType> RifReaderPressureDepthData::availableWellLogChannels( const QString& wellName )
{
    if ( m_pressureDepthDataItems.empty() )
    {
        load();
    }

    if ( !m_pressureDepthDataItems.empty() )
    {
        return { RifEclipseRftAddress::RftWellLogChannelType::TVD, RifEclipseRftAddress::RftWellLogChannelType::PRESSURE };
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RifReaderPressureDepthData::wellNames()
{
    if ( m_pressureDepthDataItems.empty() )
    {
        load();
    }

    std::set<QString> wellNames;
    for ( const RigPressureDepthData& pressureDepthData : m_pressureDepthDataItems )
    {
        wellNames.insert( pressureDepthData.wellName() );
    }
    return wellNames;
}
