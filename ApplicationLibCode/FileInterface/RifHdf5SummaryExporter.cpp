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

#include "RifHdf5Exporter.h"
#include "RifSummaryReaderInterface.h"

#include "opm/common/utility/TimeService.hpp"
#include "opm/io/eclipse/ESmry.hpp"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifHdf5SummaryExporter::ensureHdf5FileIsCreated( const std::string& smspecFileName, const std::string& h5FileName )
{
    if ( !QFile::exists( QString::fromStdString( smspecFileName ) ) ) return false;

    // TODO: Use time stamp of file to make sure the smspec file is older than the h5 file
    if ( !QFile::exists( QString::fromStdString( h5FileName ) ) )
    {
        Opm::EclIO::ESmry sourceSummaryData( smspecFileName );

        RifHdf5Exporter exporter( h5FileName );

        writeGeneralSection( exporter, sourceSummaryData );
        writeSummaryVectors( exporter, sourceSummaryData );
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

    for ( const auto& nodesForKeyword : mapKeywordToSummaryNodeIndex )
    {
        std::string keyword = nodesForKeyword.first;

        auto keywordGroup = exporter.createGroup( &summaryVectorsGroup, keyword );

        for ( auto nodeIndex : nodesForKeyword.second )
        {
            auto    summaryNode        = summaryNodeList[nodeIndex];
            auto    smspecKeywordIndex = summaryNode.smspecKeywordIndex;
            QString smspecKeywordText  = QString( "%1" ).arg( smspecKeywordIndex );

            auto dataValuesGroup             = exporter.createGroup( &keywordGroup, smspecKeywordText.toStdString() );
            const std::vector<float>& values = sourceSummaryData.get( summaryNode );

            exporter.writeDataset( dataValuesGroup, datasetName, values );
        }
    }

    return true;
}
