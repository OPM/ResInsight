/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022- Equinor ASA
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

#include "RifReaderOpmRft.h"

#include "RiaLogging.h"

#include "opm/io/eclipse/ERft.hpp"

#include "cafVecIjk.h"

#include "RiaStdStringTools.h"
#include <iomanip>
#include <iostream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderOpmRft::RifReaderOpmRft( const QString& fileName )
{
    try
    {
        m_opm_rft = std::make_unique<Opm::EclIO::ERft>( fileName.toStdString() );

        buildMetaData();
    }
    catch ( ... )
    {
        RiaLogging::error( QString( "Failed to open RFT file %1" ).arg( fileName ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseRftAddress> RifReaderOpmRft::eclipseRftAddresses()
{
    return m_addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmRft::values( const RifEclipseRftAddress& rftAddress, std::vector<double>* values )
{
    auto wellName   = rftAddress.wellName().toStdString();
    auto resultName = rftAddress.resultName().toStdString();

    auto qDate = rftAddress.timeStep().date();
    int  y     = qDate.year();
    int  m     = qDate.month();
    int  d     = qDate.day();

    if ( rftAddress.wellLogChannel() == RifEclipseRftAddress::RftWellLogChannelType::SEGMENT_VALUES )
    {
        if ( rftAddress.resultName() == RifEclipseRftAddress::segmentNumberResultName() )
        {
            auto key = std::make_pair( wellName, RftDate{ y, m, d } );

            auto segment = m_rftWellDateSegments2[key];
            for ( const auto& s : segment.topology() )
            {
                values->push_back( s.segNo() );
            }
        }
    }

    if ( resultName.empty() )
    {
        resultName = RifReaderOpmRft::resultNameFromChannelType( rftAddress.wellLogChannel() );
    }

    try
    {
        auto data = m_opm_rft->getRft<float>( resultName, wellName, y, m, d );
        if ( !data.empty() )
        {
            values->insert( values->end(), data.begin(), data.end() );
        }
    }
    catch ( ... )
    {
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QDateTime> RifReaderOpmRft::availableTimeSteps( const QString& wellName )
{
    std::set<QDateTime> timeSteps;

    for ( const auto& address : m_addresses )
    {
        if ( address.wellName() == wellName )
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
    RifReaderOpmRft::availableTimeSteps( const QString&                                     wellName,
                                         const RifEclipseRftAddress::RftWellLogChannelType& wellLogChannelName )
{
    std::set<QDateTime> timeSteps;

    for ( const auto& address : m_addresses )
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
    RifReaderOpmRft::availableTimeSteps( const QString&                                               wellName,
                                         const std::set<RifEclipseRftAddress::RftWellLogChannelType>& relevantChannels )
{
    std::set<QDateTime> timeSteps;

    for ( const auto& address : m_addresses )
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
std::set<RifEclipseRftAddress::RftWellLogChannelType> RifReaderOpmRft::availableWellLogChannels( const QString& wellName )
{
    std::set<RifEclipseRftAddress::RftWellLogChannelType> types;

    for ( const auto& a : m_addresses )
    {
        if ( a.wellName() == wellName )
        {
            if ( a.wellLogChannel() != RifEclipseRftAddress::RftWellLogChannelType::NONE )
            {
                types.insert( a.wellLogChannel() );
            }
        }
    }

    return types;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RifReaderOpmRft::wellNames()
{
    return m_wellNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmRft::cellIndices( const RifEclipseRftAddress& rftAddress, std::vector<caf::VecIjk>* indices )
{
    auto wellName = rftAddress.wellName().toStdString();

    auto date = rftAddress.timeStep().date();
    int  y    = date.year();
    int  m    = date.month();
    int  d    = date.day();

    try
    {
        auto resultNameI = "CONIPOS";
        auto dataI       = m_opm_rft->getRft<int>( resultNameI, wellName, y, m, d );

        auto resultNameJ = "CONJPOS";
        auto dataJ       = m_opm_rft->getRft<int>( resultNameJ, wellName, y, m, d );

        auto resultNameK = "CONKPOS";
        auto dataK       = m_opm_rft->getRft<int>( resultNameK, wellName, y, m, d );

        if ( !dataI.empty() && ( dataI.size() == dataJ.size() ) && ( dataI.size() == dataK.size() ) )
        {
            for ( size_t n = 0; n < dataI.size(); n++ )
            {
                // NB: Transform to zero-based cell indices
                indices->push_back( caf::VecIjk( dataI[n] - 1, dataJ[n] - 1, dataK[n] - 1 ) );
            }
        }
    }
    catch ( ... )
    {
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmRft::buildMetaData()
{
    // TODO: Assert better than return?
    if ( !isOpen() ) return;

    {
        auto wellNames = m_opm_rft->listOfWells();
        for ( const auto& w : wellNames )
        {
            m_wellNames.insert( QString::fromStdString( w ) );
        }
    }

    auto reports = m_opm_rft->listOfRftReports();
    for ( const auto& report : reports )
    {
        auto [wellName, reportDate, reportTime] = report;
        auto rftVectors                         = m_opm_rft->listOfRftArrays( wellName, reportDate );

        for ( const auto& rftVec : rftVectors )
        {
            auto [resultDataName, arrType, itemCount] = rftVec;

            int y = std::get<0>( reportDate );
            int m = std::get<1>( reportDate );
            int d = std::get<2>( reportDate );

            QDateTime dateTime;
            dateTime.setDate( QDate( y, m, d ) );

            auto channelTypes = identifyChannelType( resultDataName );
            if ( channelTypes != RifEclipseRftAddress::RftWellLogChannelType::NONE )
            {
                auto adr = RifEclipseRftAddress( QString::fromStdString( wellName ), dateTime, channelTypes );
                adr.setResultName( QString::fromStdString( resultDataName ) );
                m_addresses.insert( adr );
            }
        }
    }

    buildSegmentData();

    // Create segment result addresses
    {
        for ( const auto& segmentWellData : m_rftWellDateSegments2 )
        {
            auto [wellName, reportDate] = segmentWellData.first;
            auto segmentData            = segmentWellData.second;

            auto resultNameAndSizes = segmentData.resultNameAndSize();

            int y = std::get<0>( reportDate );
            int m = std::get<1>( reportDate );
            int d = std::get<2>( reportDate );

            QDateTime dateTime;
            dateTime.setDate( QDate( y, m, d ) );

            for ( const auto& resultNameAndSize : resultNameAndSizes )
            {
                auto adr = RifEclipseRftAddress( QString::fromStdString( wellName ),
                                                 dateTime,
                                                 RifEclipseRftAddress::RftWellLogChannelType::SEGMENT_VALUES );

                auto resultName = std::get<0>( resultNameAndSize );

                adr.setResultName( QString::fromStdString( resultName ) );

                m_addresses.insert( adr );
            }

            auto adr = RifEclipseRftAddress( QString::fromStdString( wellName ),
                                             dateTime,
                                             RifEclipseRftAddress::RftWellLogChannelType::SEGMENT_VALUES );
            adr.setResultName( RifEclipseRftAddress::segmentNumberResultName() );
            m_addresses.insert( adr );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmRft::buildSegmentData()
{
    m_rftWellDateSegments2.clear();

    auto wells = m_opm_rft->listOfWells();
    auto dates = m_opm_rft->listOfdates();

    for ( const auto& well : wells )
    {
        for ( const auto& date : dates )
        {
            std::vector<RftSegmentData> segmentsForWellDate;

            std::vector<int> segnxt;
            std::vector<int> segbrno;
            std::vector<int> brnstValues;
            std::vector<int> brnenValues;
            std::vector<int> segNo;

            {
                std::string resultName = "SEGNXT";
                if ( m_opm_rft->hasArray( resultName, well, date ) )
                {
                    segnxt = m_opm_rft->getRft<int>( resultName, well, date );
                }
            }
            {
                std::string resultName = "SEGBRNO";
                if ( m_opm_rft->hasArray( resultName, well, date ) )
                {
                    segbrno = m_opm_rft->getRft<int>( resultName, well, date );
                }
            }
            {
                std::string resultName = "BRNST";
                if ( m_opm_rft->hasArray( resultName, well, date ) )
                {
                    brnstValues = m_opm_rft->getRft<int>( resultName, well, date );
                }
            }
            {
                std::string resultName = "BRNEN";
                if ( m_opm_rft->hasArray( resultName, well, date ) )
                {
                    brnenValues = m_opm_rft->getRft<int>( resultName, well, date );
                }
            }

            if ( segnxt.empty() ) continue;
            if ( segnxt.size() != segbrno.size() ) continue;
            if ( brnenValues.empty() || brnstValues.empty() ) continue;

            for ( size_t i = 0; i < segnxt.size(); i++ )
            {
                int branchIndex     = segbrno[i] - 1;
                int nextBranchIndex = -1;
                if ( i + 1 < segbrno.size() ) nextBranchIndex = segbrno[i + 1] - 1;

                bool isLastSegmentOnBranch = branchIndex != nextBranchIndex;

                int brnst = brnstValues[branchIndex];
                int brnen = brnenValues[branchIndex];

                int segmentId = -1;
                if ( !isLastSegmentOnBranch )
                {
                    if ( i + 1 < segnxt.size() ) segmentId = segnxt[i + 1];
                }
                else
                {
                    segmentId = brnen;
                }

                segNo.push_back( segmentId );

                segmentsForWellDate.emplace_back( RftSegmentData( segnxt[i], segbrno[i], brnst, brnen, segmentId ) );
            }

            if ( segmentsForWellDate.empty() ) continue;

            RftSegment segment;
            segment.setSegmentData( segmentsForWellDate );

            auto arraysAtWellDate = m_opm_rft->listOfRftArrays( well, date );
            for ( const auto& rftResultMetaData : arraysAtWellDate )
            {
                auto [name, arrayType, size] = rftResultMetaData;
                if ( name.find( "SEG" ) == 0 )
                {
                    segment.addResultNameAndSize( rftResultMetaData );
                }
            }

            auto wellDateKey = std::make_pair( well, date );

            m_rftWellDateSegments2[wellDateKey] = segment;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmRft::segmentDataDebugLog() const
{
    for ( const auto& a : m_rftWellDateSegments2 )
    {
        auto [wellName, date] = a.first;
        auto segmentData      = a.second;

        std::cout << "\nWell: " << wellName << "Date : " << std::get<0>( date ) << " " << std::get<1>( date ) << " "
                  << std::get<2>( date ) << " \n";

        for ( const auto& r : segmentData.topology() )
        {
            std::cout << "SEGNXT  " << std::setw( 2 ) << r.segNext() << ", ";
            std::cout << "SEGBRNO " << std::setw( 2 ) << r.segBrno() << ", ";
            std::cout << "BNRST   " << std::setw( 2 ) << r.segBrnst() << ", ";
            std::cout << "BRNEN   " << std::setw( 2 ) << r.segBrnen() << ", ";
            std::cout << "SEGNO   " << std::setw( 2 ) << r.segNo() << "\n";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderOpmRft::isOpen() const
{
    return m_opm_rft != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifEclipseRftAddress::RftWellLogChannelType RifReaderOpmRft::identifyChannelType( const std::string& resultName )
{
    if ( resultName == "DEPTH" ) return RifEclipseRftAddress::RftWellLogChannelType::TVD;
    if ( resultName == "PRESSURE" ) return RifEclipseRftAddress::RftWellLogChannelType::PRESSURE;
    if ( resultName == "SWAT" ) return RifEclipseRftAddress::RftWellLogChannelType::SWAT;
    if ( resultName == "SOIL" ) return RifEclipseRftAddress::RftWellLogChannelType::SOIL;
    if ( resultName == "SGAS" ) return RifEclipseRftAddress::RftWellLogChannelType::SGAS;
    if ( resultName == "WRAT" ) return RifEclipseRftAddress::RftWellLogChannelType::WRAT;
    if ( resultName == "ORAT" ) return RifEclipseRftAddress::RftWellLogChannelType::ORAT;
    if ( resultName == "GRAT" ) return RifEclipseRftAddress::RftWellLogChannelType::GRAT;

    return RifEclipseRftAddress::RftWellLogChannelType::NONE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifReaderOpmRft::resultNameFromChannelType( RifEclipseRftAddress::RftWellLogChannelType channelType )
{
    if ( channelType == RifEclipseRftAddress::RftWellLogChannelType::TVD ) return "DEPTH";
    if ( channelType == RifEclipseRftAddress::RftWellLogChannelType::PRESSURE ) return "PRESSURE";
    if ( channelType == RifEclipseRftAddress::RftWellLogChannelType::SWAT ) return "SWAT";
    if ( channelType == RifEclipseRftAddress::RftWellLogChannelType::SOIL ) return "SOIL";
    if ( channelType == RifEclipseRftAddress::RftWellLogChannelType::SGAS ) return "SGAS";
    if ( channelType == RifEclipseRftAddress::RftWellLogChannelType::WRAT ) return "WRAT";
    if ( channelType == RifEclipseRftAddress::RftWellLogChannelType::ORAT ) return "ORAT";
    if ( channelType == RifEclipseRftAddress::RftWellLogChannelType::GRAT ) return "GRAT";

    return {};
}
