/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017  Statoil ASA
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

#include "RifReaderEclipseRft.h"

#include "RiaLogging.h"
#include "RiaQDateTimeTools.h"

#include "RiaStringEncodingTools.h"

#include "cafVecIjk.h"

#include "ert/ecl/ecl_rft_file.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderEclipseRft::RifReaderEclipseRft( const QString& fileName )
    : m_fileName( fileName )
    , m_ecl_rft_file( nullptr )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderEclipseRft::~RifReaderEclipseRft()
{
    if ( m_ecl_rft_file )
    {
        ecl_rft_file_free( m_ecl_rft_file );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseRft::open()
{
    if ( m_fileName.isEmpty() ) return;

    RiaLogging::info( QString( "Opening file '%1'" ).arg( m_fileName ) );

    m_ecl_rft_file = ecl_rft_file_alloc_case( RiaStringEncodingTools::toNativeEncoded( m_fileName ).data() );

    if ( m_ecl_rft_file == nullptr )
    {
        RiaLogging::warning( QString( "Libecl could not find/open file '%'" ).arg( m_fileName ) );
        return;
    }

    int fileSize = ecl_rft_file_get_size( m_ecl_rft_file );

    m_eclipseRftAddresses.clear();

    for ( int i = 0; i < fileSize; i++ )
    {
        ecl_rft_node_type* node = ecl_rft_file_iget_node( m_ecl_rft_file, i );

        std::string wellNameStdString = ecl_rft_node_get_well_name( node );
        QString     wellName( wellNameStdString.c_str() );
        m_wellNames.insert( wellName );

        time_t timeStepTime_t = ecl_rft_node_get_date( node );

        QDateTime timeStep = RiaQDateTimeTools::createUtcDateTime();
        timeStep.setTime_t( timeStepTime_t );

        RifEclipseRftAddress addressPressure( wellName, timeStep, RifEclipseRftAddress::PRESSURE );
        m_eclipseRftAddresses.insert( addressPressure );
        m_rftAddressToLibeclNodeIdx[addressPressure] = i;

        RifEclipseRftAddress addressDepth( wellName, timeStep, RifEclipseRftAddress::TVD );
        m_eclipseRftAddresses.insert( addressDepth );
        m_rftAddressToLibeclNodeIdx[addressDepth] = i;

        if ( ecl_rft_node_is_RFT( node ) )
        {
            RifEclipseRftAddress addressSwat( wellName, timeStep, RifEclipseRftAddress::SWAT );
            m_eclipseRftAddresses.insert( addressSwat );
            m_rftAddressToLibeclNodeIdx[addressSwat] = i;

            RifEclipseRftAddress addressSoil( wellName, timeStep, RifEclipseRftAddress::SOIL );
            m_eclipseRftAddresses.insert( addressSoil );
            m_rftAddressToLibeclNodeIdx[addressSoil] = i;

            RifEclipseRftAddress addressSgas( wellName, timeStep, RifEclipseRftAddress::SGAS );
            m_eclipseRftAddresses.insert( addressSgas );
            m_rftAddressToLibeclNodeIdx[addressSgas] = i;
        }
        else if ( ecl_rft_node_is_PLT( node ) )
        {
            RifEclipseRftAddress addressWrat( wellName, timeStep, RifEclipseRftAddress::WRAT );
            m_eclipseRftAddresses.insert( addressWrat );
            m_rftAddressToLibeclNodeIdx[addressWrat] = i;

            RifEclipseRftAddress addressOrat( wellName, timeStep, RifEclipseRftAddress::ORAT );
            m_eclipseRftAddresses.insert( addressOrat );
            m_rftAddressToLibeclNodeIdx[addressOrat] = i;

            RifEclipseRftAddress addressGrat( wellName, timeStep, RifEclipseRftAddress::GRAT );
            m_eclipseRftAddresses.insert( addressGrat );
            m_rftAddressToLibeclNodeIdx[addressGrat] = i;
        }
    }

    return;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseRftAddress> RifReaderEclipseRft::eclipseRftAddresses()
{
    if ( !m_ecl_rft_file )
    {
        open();
    }

    return m_eclipseRftAddresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseRft::values( const RifEclipseRftAddress& rftAddress, std::vector<double>* values )
{
    CVF_ASSERT( values );

    if ( !m_ecl_rft_file )
    {
        open();
    }

    values->clear();

    int index = indexFromAddress( rftAddress );
    if ( index < 0 ) return;

    ecl_rft_node_type* node = ecl_rft_file_iget_node( m_ecl_rft_file, index );

    RifEclipseRftAddress::RftWellLogChannelType wellLogChannelName = rftAddress.wellLogChannel();

    switch ( wellLogChannelName )
    {
        case RifEclipseRftAddress::TVD:
        {
            for ( int i = 0; i < ecl_rft_node_get_size( node ); i++ )
            {
                values->push_back( ecl_rft_node_iget_depth( node, i ) );
            }
            break;
        }
        case RifEclipseRftAddress::PRESSURE:
        {
            for ( int i = 0; i < ecl_rft_node_get_size( node ); i++ )
            {
                values->push_back( ecl_rft_node_iget_pressure( node, i ) );
            }
            break;
        }
        case RifEclipseRftAddress::SWAT:
        {
            for ( int i = 0; i < ecl_rft_node_get_size( node ); i++ )
            {
                values->push_back( ecl_rft_node_iget_swat( node, i ) );
            }
            break;
        }
        case RifEclipseRftAddress::SOIL:
        {
            for ( int i = 0; i < ecl_rft_node_get_size( node ); i++ )
            {
                values->push_back( ecl_rft_node_iget_soil( node, i ) );
            }
            break;
        }
        case RifEclipseRftAddress::SGAS:
        {
            for ( int i = 0; i < ecl_rft_node_get_size( node ); i++ )
            {
                values->push_back( ecl_rft_node_iget_sgas( node, i ) );
            }
            break;
        }
        case RifEclipseRftAddress::WRAT:
        {
            for ( int i = 0; i < ecl_rft_node_get_size( node ); i++ )
            {
                values->push_back( ecl_rft_node_iget_wrat( node, i ) );
            }
            break;
        }
        case RifEclipseRftAddress::ORAT:
        {
            for ( int i = 0; i < ecl_rft_node_get_size( node ); i++ )
            {
                values->push_back( ecl_rft_node_iget_orat( node, i ) );
            }
            break;
        }
        case RifEclipseRftAddress::GRAT:
        {
            for ( int i = 0; i < ecl_rft_node_get_size( node ); i++ )
            {
                values->push_back( ecl_rft_node_iget_grat( node, i ) );
            }
            break;
        }
        default:
            break;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseRft::cellIndices( const RifEclipseRftAddress& rftAddress, std::vector<caf::VecIjk>* indices )
{
    CVF_ASSERT( indices );

    if ( !m_ecl_rft_file )
    {
        open();
    }

    indices->clear();

    int index = indexFromAddress( rftAddress );
    if ( index < 0 ) return;

    ecl_rft_node_type* node = ecl_rft_file_iget_node( m_ecl_rft_file, index );

    for ( int cellIdx = 0; cellIdx < ecl_rft_node_get_size( node ); cellIdx++ )
    {
        int i, j, k;
        ecl_rft_node_iget_ijk( node, cellIdx, &i, &j, &k );

        caf::VecIjk ijk( (size_t)i, (size_t)j, (size_t)k );
        indices->push_back( ijk );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QDateTime>
    RifReaderEclipseRft::availableTimeSteps( const QString&                                     wellName,
                                             const RifEclipseRftAddress::RftWellLogChannelType& wellLogChannelName )
{
    if ( !m_ecl_rft_file )
    {
        open();
    }

    std::set<QDateTime> timeSteps;

    if ( wellName == "" ) return timeSteps;

    for ( const RifEclipseRftAddress& address : m_eclipseRftAddresses )
    {
        if ( address.wellName() == wellName && address.wellLogChannel() == wellLogChannelName )
        {
            timeSteps.insert( address.timeStep() );
        }
    }
    return timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QDateTime>
    RifReaderEclipseRft::availableTimeSteps( const QString&                                               wellName,
                                             const std::set<RifEclipseRftAddress::RftWellLogChannelType>& relevantChannels )
{
    if ( !m_ecl_rft_file )
    {
        open();
    }

    std::set<QDateTime> timeSteps;

    if ( wellName == "" ) return timeSteps;

    for ( const RifEclipseRftAddress& address : m_eclipseRftAddresses )
    {
        if ( address.wellName() == wellName && relevantChannels.count( address.wellLogChannel() ) )
        {
            timeSteps.insert( address.timeStep() );
        }
    }
    return timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QDateTime> RifReaderEclipseRft::availableTimeSteps( const QString& wellName )
{
    auto wellLogChannels = availableWellLogChannels( wellName );
    return availableTimeSteps( wellName, wellLogChannels );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseRftAddress::RftWellLogChannelType> RifReaderEclipseRft::availableWellLogChannels( const QString& wellName )
{
    if ( !m_ecl_rft_file )
    {
        open();
    }

    std::set<RifEclipseRftAddress::RftWellLogChannelType> wellLogChannelNames;

    if ( wellName == "" ) return wellLogChannelNames;

    bool pressureFound = false;
    bool rftFound      = false;
    bool pltFound      = false;

    for ( const RifEclipseRftAddress& address : m_eclipseRftAddresses )
    {
        if ( address.wellName() == wellName )
        {
            RifEclipseRftAddress::RftWellLogChannelType name = address.wellLogChannel();

            if ( !pressureFound )
            {
                if ( name == RifEclipseRftAddress::PRESSURE )
                {
                    pressureFound = true;
                    if ( rftFound && pltFound ) break;
                }
            }

            if ( !rftFound )
            {
                if ( name == RifEclipseRftAddress::RftWellLogChannelType::SWAT )
                {
                    rftFound = true;
                    if ( pltFound && pressureFound ) break;
                    continue;
                }
            }

            if ( !pltFound )
            {
                if ( name == RifEclipseRftAddress::RftWellLogChannelType::WRAT )
                {
                    pltFound = true;
                    if ( rftFound && pressureFound ) break;
                }
            }
        }
    }

    if ( pressureFound )
    {
        wellLogChannelNames.insert( RifEclipseRftAddress::PRESSURE );
    }
    if ( rftFound )
    {
        wellLogChannelNames.insert( RifEclipseRftAddress::RftWellLogChannelType::SWAT );
        wellLogChannelNames.insert( RifEclipseRftAddress::RftWellLogChannelType::SOIL );
        wellLogChannelNames.insert( RifEclipseRftAddress::RftWellLogChannelType::SGAS );
    }
    if ( pltFound )
    {
        wellLogChannelNames.insert( RifEclipseRftAddress::RftWellLogChannelType::WRAT );
        wellLogChannelNames.insert( RifEclipseRftAddress::RftWellLogChannelType::ORAT );
        wellLogChannelNames.insert( RifEclipseRftAddress::RftWellLogChannelType::GRAT );
    }

    return wellLogChannelNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RifReaderEclipseRft::wellNames()
{
    if ( !m_ecl_rft_file )
    {
        open();
    }

    return m_wellNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseRft::wellHasRftData( QString wellName )
{
    if ( !m_ecl_rft_file )
    {
        open();
    }

    for ( const QString& name : wellNames() )
    {
        if ( name == wellName )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifReaderEclipseRft::indexFromAddress( const RifEclipseRftAddress& rftAddress ) const
{
    auto it = m_rftAddressToLibeclNodeIdx.find( rftAddress );

    if ( it != m_rftAddressToLibeclNodeIdx.end() )
    {
        return it->second;
    }

    return -1;
}
