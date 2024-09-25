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
#include "RiaQDateTimeTools.h"
#include "RiaRftDefines.h"
#include "RiaStdStringTools.h"

#include "RiaOpmParserTools.h"

#include "opm/input/eclipse/Parser/Parser.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/W.hpp"
#include "opm/io/eclipse/ERft.hpp"

#include "cafAssert.h"
#include "cafVecIjk.h"

#include <iomanip>
#include <iostream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderOpmRft::RifReaderOpmRft( const QString& fileName, const QString& dataDeckFileName )
    : m_fileName( fileName )
    , m_dataDeckFileName( dataDeckFileName )
    , m_detectedErrorWhenOpeningRftFile( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderOpmRft::RifReaderOpmRft( const QString& fileName )
    : m_fileName( fileName )
    , m_detectedErrorWhenOpeningRftFile( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseRftAddress> RifReaderOpmRft::eclipseRftAddresses()
{
    openFiles();

    return m_addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmRft::values( const RifEclipseRftAddress& rftAddress, std::vector<double>* values )
{
    if ( !openFiles() ) return;

    auto wellName   = rftAddress.wellName().toStdString();
    auto resultName = rftAddress.segmentResultName().toStdString();

    auto qDate = rftAddress.timeStep().date();
    int  y     = qDate.year();
    int  m     = qDate.month();
    int  d     = qDate.day();

    if ( rftAddress.wellLogChannel() == RifEclipseRftAddress::RftWellLogChannelType::SEGMENT_VALUES )
    {
        auto key     = std::make_pair( wellName, RftDate{ y, m, d } );
        auto segment = m_rftWellDateSegments[key];

        if ( rftAddress.segmentResultName() == RiaDefines::segmentNumberResultName() )
        {
            auto data = segment.topology();

            std::vector<size_t> nonContinuousSegmentIndices;
            if ( rftAddress.segmentBranchType() == RiaDefines::RftBranchType::RFT_DEVICE )
            {
                nonContinuousSegmentIndices = segment.nonContinuousDeviceSegmentIndices( rftAddress.segmentBranchIndex() );
            }

            auto indices = segment.segmentIndicesForBranchIndex( rftAddress.segmentBranchIndex(), rftAddress.segmentBranchType() );
            for ( const auto& i : indices )
            {
                CAF_ASSERT( i < data.size() );

                values->push_back( data[i].segNo() );

                if ( std::find( nonContinuousSegmentIndices.begin(), nonContinuousSegmentIndices.end(), i ) !=
                     nonContinuousSegmentIndices.end() )
                {
                    // Use the same segment number for the dummy segment as the previous segment
                    values->push_back( values->back() );
                }
            }
            return;
        }
        else if ( rftAddress.segmentResultName() == RiaDefines::segmentBranchNumberResultName() )
        {
            auto branchNumbers = segment.tubingBranchIds();
            for ( const auto& branchNumber : branchNumbers )
            {
                values->push_back( branchNumber );
            }
            return;
        }
        else if ( rftAddress.segmentResultName() == RiaDefines::segmentConnectionMeasuredDepthResultName() )
        {
            auto segmentConnectionValues = segmentConnectionStartEndMeasuredDepth( rftAddress );
            for ( const auto& [startMD, endMD, isValidSegment] : segmentConnectionValues )
            {
                values->push_back( endMD );
            }

            return;
        }
    }

    if ( resultName.empty() )
    {
        resultName = RifReaderOpmRft::resultNameFromChannelType( rftAddress.wellLogChannel() );
    }

    try
    {
        std::vector<float> data = resultAsFloat( resultName, wellName, y, m, d );

        if ( !data.empty() )
        {
            if ( rftAddress.wellLogChannel() == RifEclipseRftAddress::RftWellLogChannelType::SEGMENT_VALUES )
            {
                auto key     = std::make_pair( wellName, RftDate{ y, m, d } );
                auto segment = m_rftWellDateSegments[key];

                if ( m_connectionResultItemCount.count( wellName ) && data.size() == m_connectionResultItemCount[wellName] )
                {
                    auto connectionValues = segmentConnectionValues( rftAddress, segment, data );
                    values->insert( values->end(), connectionValues.begin(), connectionValues.end() );
                }
                else
                {
                    std::vector<size_t> nonContinuousDeviceSegmentIndices;
                    if ( rftAddress.segmentBranchType() == RiaDefines::RftBranchType::RFT_DEVICE )
                    {
                        nonContinuousDeviceSegmentIndices = segment.nonContinuousDeviceSegmentIndices( rftAddress.segmentBranchIndex() );
                    }

                    auto indices = segment.segmentIndicesForBranchIndex( rftAddress.segmentBranchIndex(), rftAddress.segmentBranchType() );
                    for ( const auto& i : indices )
                    {
                        CAF_ASSERT( i < data.size() );
                        auto dataValue = data[i];

                        if ( std::find( nonContinuousDeviceSegmentIndices.begin(), nonContinuousDeviceSegmentIndices.end(), i ) !=
                             nonContinuousDeviceSegmentIndices.end() )
                        {
                            if ( rftAddress.segmentResultName() == RiaDefines::segmentEndDepthResultName() )
                            {
                                // Insert a depth value for segments that are not continuous. When infinity is assigned to this measured
                                // depth, no curve is drawn for this segment
                                values->push_back( dataValue - 0.1 );
                            }
                            else
                            {
                                // Use infinity to make sure no curve is drawn for this segment
                                values->push_back( std::numeric_limits<double>::infinity() );
                            }
                        }

                        values->push_back( dataValue );
                    }
                }

                if ( resultName == "CONFAC" || resultName == "CONKH" )
                {
                    // Replace undefined values with zero to improve readability of plots
                    std::replace( data.begin(), data.end(), std::numeric_limits<double>::infinity(), 0.0 );
                }
            }
            else
            {
                values->insert( values->end(), data.begin(), data.end() );
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
std::set<QDateTime> RifReaderOpmRft::availableTimeSteps( const QString& wellName )
{
    openFiles();

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
std::set<QDateTime> RifReaderOpmRft::availableTimeSteps( const QString&                                     wellName,
                                                         const RifEclipseRftAddress::RftWellLogChannelType& wellLogChannelName )
{
    openFiles();

    if ( wellLogChannelName == RifEclipseRftAddress::RftWellLogChannelType::SEGMENT_VALUES ) return m_rftSegmentTimeSteps;

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
std::set<QDateTime> RifReaderOpmRft::availableTimeSteps( const QString&                                               wellName,
                                                         const std::set<RifEclipseRftAddress::RftWellLogChannelType>& relevantChannels )
{
    openFiles();

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
    openFiles();

    std::set<RifEclipseRftAddress::RftWellLogChannelType> types;

    for ( const auto& a : m_addresses )
    {
        if ( ( a.wellName() == wellName ) && ( a.wellLogChannel() != RifEclipseRftAddress::RftWellLogChannelType::NONE ) )
        {
            types.insert( a.wellLogChannel() );
        }
    }

    return types;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RifReaderOpmRft::wellNames()
{
    openFiles();

    return m_wellNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::VecIjk> RifReaderOpmRft::cellIndices( const QString& wellName, const QDateTime& timeStep )
{
    if ( !openFiles() ) return {};

    auto stdWellName = wellName.toStdString();

    auto date = timeStep.date();
    int  y    = date.year();
    int  m    = date.month();
    int  d    = date.day();

    std::vector<caf::VecIjk> indices;

    try
    {
        auto dataI = m_opm_rft->getRft<int>( RiaDefines::segmentConnectionIPos(), stdWellName, y, m, d );
        auto dataJ = m_opm_rft->getRft<int>( RiaDefines::segmentConnectionJPos(), stdWellName, y, m, d );
        auto dataK = m_opm_rft->getRft<int>( RiaDefines::segmentConnectionKPos(), stdWellName, y, m, d );

        if ( !dataI.empty() && ( dataI.size() == dataJ.size() ) && ( dataI.size() == dataK.size() ) )
        {
            for ( size_t n = 0; n < dataI.size(); n++ )
            {
                // NB: Transform to zero-based cell indices
                indices.push_back( caf::VecIjk( dataI[n] - 1, dataJ[n] - 1, dataK[n] - 1 ) );
            }
        }
    }
    catch ( ... )
    {
    }

    return indices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<int, int>
    RifReaderOpmRft::branchIdsAndOneBasedIndices( const QString& wellName, const QDateTime& timeStep, RiaDefines::RftBranchType branchType )
{
    int y = timeStep.date().year();
    int m = timeStep.date().month();
    int d = timeStep.date().day();

    auto key = std::make_pair( wellName.toStdString(), RftDate{ y, m, d } );
    if ( m_rftWellDateSegments.count( key ) > 0 )
    {
        auto segment = m_rftWellDateSegments[key];
        return segment.branchIdsAndOneBasedBranchIndices( branchType );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifRftSegment RifReaderOpmRft::segmentForWell( const QString& wellName, const QDateTime& timeStep )
{
    openFiles();

    int y = timeStep.date().year();
    int m = timeStep.date().month();
    int d = timeStep.date().day();

    auto key = std::make_pair( wellName.toStdString(), RftDate{ y, m, d } );
    if ( m_rftWellDateSegments.count( key ) > 0 )
    {
        return m_rftWellDateSegments[key];
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmRft::readWseglink( const std::string& filePath )
{
    if ( filePath.empty() ) return;

    QString text = QString( "Scanning for WSEGLINK data in %1\n" ).arg( QString::fromStdString( filePath ) );

    m_wseglink = RiaOpmParserTools::extractWseglink( filePath );
    if ( !m_wseglink.empty() )
    {
        text += "Imported WSEGLINK data from well(s):\n";

        for ( auto [wellName, links] : m_wseglink )
        {
            text += "  " + QString::fromStdString( wellName ) + "\n";
        }
    }
    else
    {
        text += QString( "  No WSEGLINK data found." );
    }

    RiaLogging::info( text );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmRft::openFiles( const QString& fileName, const QString& dataDeckFileName )
{
    try
    {
        m_opm_rft = std::make_unique<Opm::EclIO::ERft>( fileName.toStdString() );

        readWseglink( dataDeckFileName.toStdString() );

        buildMetaData();

        return;
    }
    catch ( const std::exception& e )
    {
        RiaLogging::error( QString( "Failed to open RFT file %1\n%2" ).arg( fileName ).arg( e.what() ) );
    }
    catch ( ... )
    {
        RiaLogging::error( QString( "Failed to open RFT file %1" ).arg( fileName ) );
    }

    m_detectedErrorWhenOpeningRftFile = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifReaderOpmRft::SegmentConnectionStartEnd>
    RifReaderOpmRft::segmentConnectionStartEndMeasuredDepth( const RifEclipseRftAddress& rftAddress )
{
    try
    {
        std::vector<RifReaderOpmRft::SegmentConnectionStartEnd> startEndValues;

        if ( !isOpen() ) return startEndValues;

        const int y = rftAddress.timeStep().date().year();
        const int m = rftAddress.timeStep().date().month();
        const int d = rftAddress.timeStep().date().day();

        const std::string wellName = rftAddress.wellName().toStdString();

        const std::string conbrnoResultName  = RiaDefines::segmentConnectionBranchNoResultName();
        const std::string conlenstResultName = RiaDefines::segmentConnectionStartDepthResultName();
        const std::string conlenenResultName = RiaDefines::segmentConnectionEndDepthResultName();

        const auto connnectionBranchNumbers = m_opm_rft->getRft<int>( conbrnoResultName, wellName, y, m, d );
        const auto startMD                  = m_opm_rft->getRft<float>( conlenstResultName, wellName, y, m, d );
        const auto endMD                    = m_opm_rft->getRft<float>( conlenenResultName, wellName, y, m, d );

        const size_t size = connnectionBranchNumbers.size();
        if ( size == startMD.size() && size == endMD.size() )
        {
            auto segment = segmentForWell( rftAddress.wellName(), rftAddress.timeStep() );

            auto branchIdIndex = segment.branchIdsAndOneBasedBranchIndices( rftAddress.segmentBranchType() );

            // Convert the branch number to the branch index
            // Filter data based on branch index

            bool isFirstSegment = true;

            for ( size_t i = 0; i < connnectionBranchNumbers.size(); i++ )
            {
                if ( branchIdIndex.count( connnectionBranchNumbers[i] ) )
                {
                    auto branchIndex = branchIdIndex[connnectionBranchNumbers[i]];
                    if ( branchIndex == rftAddress.segmentBranchIndex() )
                    {
                        if ( !isFirstSegment && std::fabs( startMD[i] - endMD[i - 1] ) > 0.1 )
                        {
                            // Insert a segment representing the connection between the segments. Assign infinity as value to this segment
                            // to allow discontinuous plotting.
                            startEndValues.emplace_back( endMD[i - 1], startMD[i], false );
                        }
                        startEndValues.emplace_back( startMD[i], endMD[i], true );
                        isFirstSegment = false;
                    }
                }
            }
        }

        return startEndValues;
    }
    catch ( ... )
    {
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<float> RifReaderOpmRft::segmentConnectionValues( const RifEclipseRftAddress& rftAddress,
                                                             const RifRftSegment&        rftSegment,
                                                             const std::vector<float>&   nativeValues )
{
    std::vector<float> branchValues;

    const int y = rftAddress.timeStep().date().year();
    const int m = rftAddress.timeStep().date().month();
    const int d = rftAddress.timeStep().date().day();

    const auto connnectionBranchNumbers =
        m_opm_rft->getRft<int>( RiaDefines::segmentConnectionBranchNoResultName(), rftAddress.wellName().toStdString(), y, m, d );

    if ( nativeValues.size() == connnectionBranchNumbers.size() )
    {
        auto branchIdIndex = rftSegment.branchIdsAndOneBasedBranchIndices( rftAddress.segmentBranchType() );

        // Convert the branch number to the branch index
        // Filter data based on branch index
        for ( size_t i = 0; i < connnectionBranchNumbers.size(); i++ )
        {
            if ( branchIdIndex.count( connnectionBranchNumbers[i] ) )
            {
                auto branchIndex = branchIdIndex[connnectionBranchNumbers[i]];
                if ( branchIndex == rftAddress.segmentBranchIndex() )
                {
                    branchValues.push_back( nativeValues[i] );
                }
            }
        }
    }

    std::vector<float> allResultValues;

    auto   segmentConnectionValues = segmentConnectionStartEndMeasuredDepth( rftAddress );
    size_t segmentConnectionIndex  = 0;
    for ( auto branchValue : branchValues )
    {
        if ( segmentConnectionIndex < segmentConnectionValues.size() )
        {
            auto [startMD, endMD, isValidSegment] = segmentConnectionValues[segmentConnectionIndex];

            if ( !isValidSegment )
            {
                // Use infinity to make sure no curve is drawn for this segment
                allResultValues.push_back( std::numeric_limits<double>::infinity() );

                segmentConnectionIndex++;
            }
            allResultValues.push_back( branchValue );
        }

        segmentConnectionIndex++;
    }

    return allResultValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmRft::buildMetaData()
{
    // TODO: Assert better than return?
    if ( !isOpen() ) return;

    importWellNames();

    auto reports = m_opm_rft->listOfRftReports();
    for ( const auto& [wellName, reportDate, reportTime] : reports )
    {
        auto results = m_opm_rft->listOfRftArrays( wellName, reportDate );

        for ( const auto& [name, arrayType, size] : results )
        {
            const auto& [y, m, d] = reportDate;

            auto dt = RiaQDateTimeTools::createUtcDateTime( QDate( y, m, d ) );

            auto channelType = identifyChannelType( name );
            if ( channelType != RifEclipseRftAddress::RftWellLogChannelType::NONE )
            {
                auto adr = RifEclipseRftAddress::createAddress( QString::fromStdString( wellName ), dt, channelType );
                m_addresses.insert( adr );
            }
        }
    }

    buildSegmentData();

    // Create segment result addresses
    for ( const auto& segmentWellData : m_rftWellDateSegments )
    {
        auto [wellName, reportDate] = segmentWellData.first;
        auto segmentData            = segmentWellData.second;

        const auto& [y, m, d] = reportDate;
        auto dt               = RiaQDateTimeTools::createUtcDateTime( QDate( y, m, d ) );

        m_rftSegmentTimeSteps.insert( dt );

        auto resultNameAndSizes = segmentData.resultNameAndSize();
        for ( const auto& [name, arrayType, size] : resultNameAndSizes )
        {
            auto adr = RifEclipseRftAddress::createSegmentAddress( QString::fromStdString( wellName ), dt, QString::fromStdString( name ) );

            m_addresses.insert( adr );
        }

        auto adr = RifEclipseRftAddress::createSegmentAddress( QString::fromStdString( wellName ), dt, RiaDefines::segmentNumberResultName() );

        m_addresses.insert( adr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmRft::buildSegmentData()
{
    m_rftWellDateSegments.clear();

    auto wellNames = m_opm_rft->listOfWells();
    auto dates     = m_opm_rft->listOfdates();

    for ( const auto& wellName : wellNames )
    {
        for ( const auto& date : dates )
        {
            std::vector<RifRftSegmentData> segmentsForWellDate;

            std::vector<int> segnxt      = importWellData( wellName, "SEGNXT", date );
            std::vector<int> segbrno     = importWellData( wellName, "SEGBRNO", date );
            std::vector<int> brnstValues = importWellData( wellName, "BRNST", date );
            std::vector<int> brnenValues = importWellData( wellName, "BRNEN", date );

            if ( segnxt.empty() ) continue;
            if ( segnxt.size() != segbrno.size() ) continue;
            if ( brnenValues.empty() || brnstValues.empty() ) continue;

            std::vector<int> segNo;
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

                segmentsForWellDate.emplace_back( RifRftSegmentData( segnxt[i], segbrno[i], brnst, brnen, segmentId ) );
            }

            if ( segmentsForWellDate.empty() ) continue;

            RifRftSegment segment;
            segment.setSegmentData( segmentsForWellDate );

            auto results = m_opm_rft->listOfRftArrays( wellName, date );

            for ( const auto& [name, arrayType, size] : results )
            {
                if ( ( RiaDefines::isSegmentResult( QString::fromStdString( name ) ) ) && m_segmentResultItemCount.count( wellName ) == 0 )
                {
                    m_segmentResultItemCount[wellName] = size;
                }
                if ( RiaDefines::isSegmentConnectionResult( QString::fromStdString( name ) ) &&
                     m_connectionResultItemCount.count( wellName ) == 0 )
                {
                    m_connectionResultItemCount[wellName] = size;
                }
            }

            for ( const auto& rftResultMetaData : results )
            {
                const auto& [name, arrayType, size] = rftResultMetaData;

                bool isResultItemCountValid = false;
                if ( m_segmentResultItemCount.count( wellName ) && size == static_cast<int64_t>( m_segmentResultItemCount[wellName] ) )
                    isResultItemCountValid = true;

                if ( m_connectionResultItemCount.count( wellName ) && size == static_cast<int64_t>( m_connectionResultItemCount[wellName] ) )
                    isResultItemCountValid = true;

                if ( isResultItemCountValid )
                {
                    segment.addResultNameAndSize( rftResultMetaData );
                }
            }

            auto wellDateKey                   = std::make_pair( wellName, date );
            m_rftWellDateSegments[wellDateKey] = segment;

            buildSegmentBranchTypes( wellDateKey );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmRft::segmentDataDebugLog() const
{
    for ( const auto& a : m_rftWellDateSegments )
    {
        auto [wellName, date] = a.first;
        auto segmentData      = a.second;

        const auto& [y, m, d] = date;

        std::cout << "\nWell: " << wellName << "Date : " << y << " " << m << " " << d << " \n";

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
void RifReaderOpmRft::importWellNames()
{
    auto names = m_opm_rft->listOfWells();
    for ( const auto& w : names )
    {
        m_wellNames.insert( QString::fromStdString( w ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmRft::buildSegmentBranchTypes( const RftSegmentKey& segmentKey )
{
    // A branch can have up to three layers of branches
    // Tubing branch
    // The inner most branch representing the tubing pipe
    //
    // Device branch
    // Layer between tubing branch and annulus branch or reservoir
    // The device segment is connected to a segment on the tubing branch
    //
    // Annulus branch
    // Layer between device branch and reservoir. The segment connection data is imported from WSEGLINK in the
    // data deck

    auto           wellName   = segmentKey.first;
    auto           date       = segmentKey.second;
    RifRftSegment& segmentRef = m_rftWellDateSegments[segmentKey];

    const auto& [y, m, d] = date;
    auto dt               = RiaQDateTimeTools::createUtcDateTime( QDate( y, m, d ) );

    std::vector<double> seglenstValues;
    std::vector<double> seglenenValues;
    {
        auto resultName = RifEclipseRftAddress::createSegmentAddress( QString::fromStdString( wellName ), dt, "SEGLENST" );

        values( resultName, &seglenstValues );

        if ( seglenstValues.size() > 2 )
        {
            seglenstValues[0] = seglenstValues[1];
        }
    }
    {
        auto resultName = RifEclipseRftAddress::createSegmentAddress( QString::fromStdString( wellName ), dt, "SEGLENEN" );

        values( resultName, &seglenenValues );
    }

    if ( !seglenenValues.empty() && !seglenstValues.empty() )
    {
        identifyTubingCandidateBranches( segmentRef, wellName, seglenstValues, seglenenValues );
        identifyAnnulusBranches( segmentRef, seglenstValues );

        // The tubing branches are given increasing branch indices. If a tubing branch is categorized as an
        // annulus branch, the index values must be reassigned. Each triplet of tubing/device/annulus has a
        // unique branch index.
        reassignBranchIndices( segmentRef );

        identifyDeviceBranches( segmentRef, seglenstValues );

        // Assign branch index to annulus branches
        auto branchIds = segmentRef.branchIds();
        for ( auto branchId : branchIds )
        {
            auto branchType = segmentRef.branchType( branchId );
            if ( branchType == RiaDefines::RftBranchType::RFT_ANNULUS )
            {
                auto segmentIndices = segmentRef.segmentIndicesForBranchNumber( branchId );
                if ( segmentIndices.empty() ) continue;

                auto firstSegmentIndex      = segmentIndices.front();
                auto firstSegment           = segmentRef.topology()[firstSegmentIndex];
                auto candidateSegmentNumber = firstSegment.segNext();

                auto candidateDeviceSeg = segmentRef.segmentData( candidateSegmentNumber );
                if ( candidateDeviceSeg )
                {
                    auto branchIndex = segmentRef.oneBasedBranchIndexForBranchId( candidateDeviceSeg->segBrno() );
                    if ( branchIndex >= 0 )
                    {
                        segmentRef.setOneBasedBranchIndex( branchId, branchIndex );
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmRft::identifyTubingCandidateBranches( RifRftSegment&             segmentRef,
                                                       const std::string&         wellName,
                                                       const std::vector<double>& seglenstValues,
                                                       const std::vector<double>& seglenenValues )
{
    int oneBasedBranchIndex = 1;

    auto branchIds = segmentRef.branchIds();
    for ( auto id : branchIds )
    {
        double minimumMD = std::numeric_limits<double>::max();
        double maximumMD = std::numeric_limits<double>::min();

        std::vector<int> segmentNumbers;

        auto indices = segmentRef.segmentIndicesForBranchNumber( id );
        for ( auto i : indices )
        {
            minimumMD = std::min( minimumMD, seglenstValues[i] );
            maximumMD = std::max( maximumMD, seglenenValues[i] );
            segmentNumbers.push_back( segmentRef.topology()[i].segNo() );
        }

        double length = maximumMD - minimumMD;

        segmentRef.setBranchLength( id, length );

        RiaDefines::RftBranchType branchType = RiaDefines::RftBranchType::RFT_UNKNOWN;

        bool hasFoundAnnulusBranch = false;

        // If WESEGLINK is imported, get annulus segments for well
        auto annulusSegments = annulusSegmentsForWell( wellName );

        std::vector<int> matchingSegments;
        std::set_intersection( segmentNumbers.begin(),
                               segmentNumbers.end(),
                               annulusSegments.begin(),
                               annulusSegments.end(),
                               std::inserter( matchingSegments, matchingSegments.end() ) );

        if ( !matchingSegments.empty() )
        {
            {
                branchType = RiaDefines::RftBranchType::RFT_ANNULUS;

                // NOTE: Assign branch index after device branch is detected
                hasFoundAnnulusBranch = true;
            }
        }

        if ( !hasFoundAnnulusBranch )
        {
            const double tubingThreshold = 1.0;
            if ( length > tubingThreshold )
            {
                branchType = RiaDefines::RftBranchType::RFT_TUBING;
                segmentRef.setOneBasedBranchIndex( id, oneBasedBranchIndex++ );
            }
        }

        segmentRef.setBranchType( id, branchType );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmRft::identifyAnnulusBranches( RifRftSegment& segmentRef, const std::vector<double>& seglenstValues )
{
    // If no WESEGLINK data is present, compare the location of the last N segments of two tubing branches. If
    // the difference is correct, mark candidate branch as annulus branch instead of tubing.

    if ( m_wseglink.empty() )
    {
        auto tubingIds = segmentRef.tubingBranchIds();

        std::map<size_t, std::vector<double>> seglenstForBranch;

        for ( auto branchId : tubingIds )
        {
            std::vector<double> values;

            auto indices = segmentRef.segmentIndicesForBranchNumber( branchId );
            for ( auto i : indices )
            {
                values.push_back( seglenstValues[i] );
            }

            seglenstForBranch[branchId] = values;
        }

        std::set<int> annulusBranchIds;

        for ( auto branchId : tubingIds )
        {
            if ( annulusBranchIds.count( branchId ) ) continue;

            for ( auto candidateBranchId : tubingIds )
            {
                if ( candidateBranchId == branchId ) continue;
                if ( annulusBranchIds.count( candidateBranchId ) ) continue;

                auto branchValues    = seglenstForBranch.at( branchId );
                auto candidateValues = seglenstForBranch.at( candidateBranchId );

                double lastBranchValue    = branchValues.back();
                double lastCandidateValue = candidateValues.back();

                double diff = lastCandidateValue - lastBranchValue;

                const double epsilon               = 1e-3;
                const double distanceTubingAnnulus = 0.1;
                if ( std::fabs( ( std::fabs( diff ) - distanceTubingAnnulus ) ) < epsilon )
                {
                    int annulusBranchId = ( diff > 0 ) ? candidateBranchId : branchId;

                    segmentRef.setBranchType( annulusBranchId, RiaDefines::RftBranchType::RFT_ANNULUS );
                    annulusBranchIds.insert( annulusBranchId );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmRft::reassignBranchIndices( RifRftSegment& segmentRef )
{
    auto tubingBranchIds = segmentRef.tubingBranchIds();

    int oneBasedBranchIndex = 1;

    std::map<int, int> newOneBasedBranchIndex;
    for ( auto branchId : tubingBranchIds )
    {
        auto previsousIndex                    = segmentRef.oneBasedBranchIndexForBranchId( branchId );
        newOneBasedBranchIndex[previsousIndex] = oneBasedBranchIndex++;
    }

    for ( auto branchId : segmentRef.branchIds() )
    {
        auto branchIndex = segmentRef.oneBasedBranchIndexForBranchId( branchId );
        if ( newOneBasedBranchIndex.count( branchIndex ) )
        {
            segmentRef.setOneBasedBranchIndex( branchId, newOneBasedBranchIndex.at( branchIndex ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmRft::identifyDeviceBranches( RifRftSegment& segmentRef, const std::vector<double>& seglenstValues )
{
    auto tubingBranchIds = segmentRef.tubingBranchIds();

    for ( auto& segment : segmentRef.topology() )
    {
        auto segmentBranchId = segment.segBrno();
        auto it              = std::find( tubingBranchIds.begin(), tubingBranchIds.end(), segmentBranchId );
        if ( it == tubingBranchIds.end() )
        {
            auto tubingSegmentNumber = segment.segNext();

            auto tubingSegmentData = segmentRef.segmentData( tubingSegmentNumber );
            if ( tubingSegmentData != nullptr )
            {
                auto it = std::find( tubingBranchIds.begin(), tubingBranchIds.end(), tubingSegmentData->segBrno() );
                if ( it != tubingBranchIds.end() )
                {
                    // Find all connected segments that is not assigned a branch type, and mark as device layer

                    auto tubingBranchIndex = segmentRef.oneBasedBranchIndexForBranchId( tubingSegmentData->segBrno() );
                    segmentRef.createDeviceBranch( segment.segNo(), tubingBranchIndex, seglenstValues );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RifReaderOpmRft::importWellData( const std::string& wellName, const std::string& propertyName, const RftDate& date ) const
{
    // PERFORMANCE NOTE
    // Use method hasRft() that do not throw exception if RFT data is not available. Using this method and avoid
    // try/catch and exceptions is way faster.
    if ( !m_opm_rft->hasRft( wellName, date ) ) return {};

    try
    {
        // The hasArray method can throw, so we must use a try/catch block here
        if ( m_opm_rft->hasArray( propertyName, wellName, date ) )
        {
            return m_opm_rft->getRft<int>( propertyName, wellName, date );
        }
    }
    catch ( ... )
    {
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<int, int>> RifReaderOpmRft::annulusLinksForWell( const std::string& wellName ) const
{
    auto it = m_wseglink.find( wellName );
    if ( it != m_wseglink.end() )
    {
        return it->second;
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<int> RifReaderOpmRft::annulusSegmentsForWell( const std::string& wellName ) const
{
    std::vector<int> annulusSegments;

    for ( auto it : annulusLinksForWell( wellName ) )
    {
        annulusSegments.push_back( it.first );
    }

    return annulusSegments;
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<float> RifReaderOpmRft::resultAsFloat( const std::string& resultName, const std::string& wellName, int year, int month, int day ) const
{
    Opm::EclIO::eclArrType resultDataType = Opm::EclIO::eclArrType::REAL;

    auto results = m_opm_rft->listOfRftArrays( wellName, year, month, day );
    for ( const auto& [name, arrayType, size] : results )
    {
        if ( resultName == name )
        {
            resultDataType = arrayType;
            break;
        }
    }

    if ( resultDataType == Opm::EclIO::eclArrType::INTE )
    {
        std::vector<float> data;

        auto integerData = m_opm_rft->getRft<int>( resultName, wellName, year, month, day );
        for ( auto val : integerData )
        {
            data.push_back( val );
        }

        return data;
    }
    else
    {
        return m_opm_rft->getRft<float>( resultName, wellName, year, month, day );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderOpmRft::openFiles()
{
    // The file open operations can be costly, so do lazy loading of the files
    // NB! Make sure that this function is called from all public functions that needs to access the files

    if ( !isOpen() && !m_detectedErrorWhenOpeningRftFile )
    {
        openFiles( m_fileName, m_dataDeckFileName );
    }

    return isOpen();
}
