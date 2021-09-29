/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

#include "RifHdf5SummaryExporter.h"

#include "RiaLogging.h"
#include "RiaPreferencesSummary.h"
#include "RiaStdStringTools.h"

#include "RifHdf5Exporter.h"
#include "RifSummaryReaderInterface.h"

#include "opm/common/utility/FileSystem.hpp"
#include "opm/common/utility/TimeService.hpp"
#include "opm/io/eclipse/ESmry.hpp"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifHdf5SummaryExporter::ensureHdf5FileIsCreatedMultithreaded( const std::vector<std::string>& smspecFileNames,
                                                                   const std::vector<std::string>& h5FileNames,
                                                                   int                             threadCount )
{
    if ( smspecFileNames.empty() ) return true;

    if ( smspecFileNames.size() != h5FileNames.size() ) return false;

    size_t hdfFilesCreatedCount = 0;

    bool useMultipleThreads = threadCount > 1;

#pragma omp parallel for schedule( dynamic ) if ( useMultipleThreads ) num_threads( threadCount )
    for ( int cIdx = 0; cIdx < static_cast<int>( smspecFileNames.size() ); ++cIdx )
    {
        const auto& smspecFileName = smspecFileNames[cIdx];
        const auto& h5FileName     = h5FileNames[cIdx];

        RifHdf5SummaryExporter::ensureHdf5FileIsCreated( smspecFileName, h5FileName, hdfFilesCreatedCount );
    }

    if ( hdfFilesCreatedCount > 0 )
    {
        QString txt = QString( "Created [ %1 ] h5 files from a total of [ %2 ] summary files" )
                          .arg( static_cast<int>( hdfFilesCreatedCount ) )
                          .arg( static_cast<int>( smspecFileNames.size() ) );
        RiaLogging::info( txt );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifHdf5SummaryExporter::ensureHdf5FileIsCreated( const std::string& smspecFileName,
                                                      const std::string& h5FileName,
                                                      size_t&            hdfFilesCreatedCount )
{
    if ( !Opm::filesystem::exists( smspecFileName ) ) return false;

    {
        // Check if we have write permission in the folder
        QFileInfo info( QString::fromStdString( smspecFileName ) );

        if ( !info.isWritable() ) return true;
    }

    bool exportIsRequired = false;

    {
        bool h5FileExists = Opm::filesystem::exists( h5FileName );
        if ( !h5FileExists ) exportIsRequired = true;

        RiaPreferencesSummary* prefs = RiaPreferencesSummary::current();
        if ( prefs->checkH5SummaryDataTimeStamp() && h5FileExists )
        {
            if ( RifHdf5SummaryExporter::isFirstOlderThanSecond( h5FileName, smspecFileName ) )
            {
                exportIsRequired = true;
            }
        }
    }

    if ( exportIsRequired )
    {
        try
        {
            Opm::EclIO::ESmry sourceSummaryData( smspecFileName );

            // Read all data summary data before starting export to HDF. Loading one and one summary vector causes huge
            // performance penalty
            sourceSummaryData.LoadData();

#pragma omp critical( critical_section_HDF5_export )
            {
                // HDF5 file access is not thread-safe, always make sure we use the HDF5 library from a single thread

                RifHdf5Exporter exporter( h5FileName );

                writeGeneralSection( exporter, sourceSummaryData );
                writeSummaryVectors( exporter, sourceSummaryData );

                hdfFilesCreatedCount++;
            }
        }
        catch ( std::exception& e )
        {
            QString txt =
                QString( "HDF export to file %1 failed : %3" ).arg( QString::fromStdString( smspecFileName ), e.what() );

            RiaLogging::error( txt );

            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifHdf5SummaryExporter::writeGeneralSection( RifHdf5Exporter& exporter, Opm::EclIO::ESmry& sourceSummaryData )
{
    auto timesteps = sourceSummaryData.dates();

    auto group = exporter.createGroup( nullptr, "general" );

    {
        std::vector<int> values( 1 );
        values[0] = -1;

        exporter.writeDataset( group, "checksum", values );
    }

    {
        auto startDate = sourceSummaryData.startdate();

        // Make sure we convert time identically as the inverse of make_date in ESmry.cpp
        time_t            firstTimeStep = std::chrono::system_clock::to_time_t( startDate );
        Opm::TimeStampUTC opmTimeStamp( firstTimeStep );

        int day   = opmTimeStamp.day();
        int month = opmTimeStamp.month();
        int year  = opmTimeStamp.year();

        int hour   = opmTimeStamp.hour();
        int minute = opmTimeStamp.minutes();
        int second = opmTimeStamp.seconds();

        std::vector<int> timeValues( 7 );
        timeValues[0] = day;
        timeValues[1] = month;
        timeValues[2] = year;
        timeValues[3] = hour;
        timeValues[4] = minute;
        timeValues[5] = second;
        timeValues[6] = 0; // Unknown value, could be millisec

        exporter.writeDataset( group, "start_date", timeValues );
    }

    {
        std::vector<int> values( 2 );
        values[0] = 1;
        values[1] = 7;

        exporter.writeDataset( group, "version", values );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifHdf5SummaryExporter::writeSummaryVectors( RifHdf5Exporter& exporter, Opm::EclIO::ESmry& sourceSummaryData )
{
    using SumNodeVector = std::vector<Opm::EclIO::SummaryNode>;

    size_t valueCount = sourceSummaryData.numberOfTimeSteps();
    if ( valueCount == 0 ) return false;

    const std::string datasetName( "values" );

    const SumNodeVector& summaryNodeList = sourceSummaryData.summaryNodeList();

    auto summaryVectorsGroup = exporter.createGroup( nullptr, "summary_vectors" );

    std::map<std::string, std::vector<size_t>> mapKeywordToSummaryNodeIndex;

    for ( size_t i = 0; i < summaryNodeList.size(); i++ )
    {
        const auto        summaryNode = summaryNodeList[i];
        const std::string keyword     = summaryNode.keyword;

        if ( mapKeywordToSummaryNodeIndex.find( keyword ) == mapKeywordToSummaryNodeIndex.end() )
        {
            mapKeywordToSummaryNodeIndex[keyword] = std::vector<size_t>();
        }

        auto it = mapKeywordToSummaryNodeIndex.find( keyword );
        if ( it != mapKeywordToSummaryNodeIndex.end() )
        {
            it->second.push_back( i );
        }
    }

    std::set<std::string> exportErrorKeywords;

    for ( const auto& nodesForKeyword : mapKeywordToSummaryNodeIndex )
    {
        std::string keyword = nodesForKeyword.first;

        auto keywordGroup = exporter.createGroup( &summaryVectorsGroup, keyword );

        for ( auto nodeIndex : nodesForKeyword.second )
        {
            const auto&    summaryNode        = summaryNodeList[nodeIndex];
            auto           smspecKeywordIndex = summaryNode.smspecKeywordIndex;
            const QString& smspecKeywordText  = QString( "%1" ).arg( smspecKeywordIndex );

            try
            {
                const std::vector<float>& values = sourceSummaryData.get( summaryNode );
                auto dataValuesGroup = exporter.createGroup( &keywordGroup, smspecKeywordText.toStdString() );

                exporter.writeDataset( dataValuesGroup, datasetName, values );
                dataValuesGroup.close();
            }
            catch ( ... )
            {
                exportErrorKeywords.insert( keyword );
            }
        }

        keywordGroup.close();
    }

    if ( !exportErrorKeywords.empty() )
    {
        std::vector<std::string> keywordVector;
        for ( const auto& k : exportErrorKeywords )
        {
            keywordVector.push_back( k );
        }
        auto txt = RiaStdStringTools::joinStrings( keywordVector, ',' );

        QString errorTxt = QString( "Failed to export keywords %1 " ).arg( QString::fromStdString( txt ) );
        RiaLogging::error( errorTxt );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifHdf5SummaryExporter::isFirstOlderThanSecond( const std::string& firstFileName, const std::string& secondFileName )
{
    // Use Opm namespace to make sure the code compiles on older compilers

    if ( !Opm::filesystem::exists( firstFileName ) || !Opm::filesystem::exists( secondFileName ) ) return false;

    auto timeA = Opm::filesystem::last_write_time( firstFileName );
    auto timeB = Opm::filesystem::last_write_time( secondFileName );

    return ( timeA < timeB );
}
