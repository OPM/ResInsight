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

    {
        std::vector<int> values( 1 );
        values[0] = -1;

        exporter.writeDataset( "general", "checksum", values );
    }

    {
        auto startDate = sourceSummaryData.startdate();

        time_t firstTimeStep = std::chrono::system_clock::to_time_t( startDate );

        QDateTime dt = QDateTime::fromTime_t( firstTimeStep );

        int day   = dt.date().day();
        int month = dt.date().month();
        int year  = dt.date().year();

        int hour   = dt.time().hour();
        int minute = dt.time().minute();
        int second = dt.time().second();

        std::vector<int> timeValues( 7 );
        timeValues[0] = day;
        timeValues[1] = month;
        timeValues[2] = year;
        timeValues[3] = hour;
        timeValues[4] = minute;
        timeValues[5] = second;
        timeValues[6] = 0; // Unknown value, could be millisec

        exporter.writeDataset( "general", "start_date", timeValues );
    }

    {
        std::vector<int> values( 2 );
        values[0] = 1;
        values[1] = 7;

        exporter.writeDataset( "general", "version", values );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifHdf5SummaryExporter::writeSummaryVectors( RifHdf5Exporter& exporter, Opm::EclIO::ESmry& sourceSummaryData )
{
    size_t valueCount = sourceSummaryData.numberOfTimeSteps();
    if ( valueCount == 0 ) return false;

    const std::string datasetName( "values" );

    const std::vector<Opm::EclIO::SummaryNode>& summaryNodeList = sourceSummaryData.summaryNodeList();

    auto summaryVectorsGroup = exporter.findOrCreateGroup( nullptr, "summary_vectors" );

    for ( const auto& summaryNode : summaryNodeList )
    {
        auto                      smspecKeywordIndex = summaryNode.smspecKeywordIndex;
        QString                   smspecKeywordText  = QString( "%1" ).arg( smspecKeywordIndex );
        const auto&               quantity           = summaryNode.keyword;
        const std::vector<float>& values             = sourceSummaryData.get( summaryNode );

        exporter.exportSummaryVector( summaryVectorsGroup, quantity, smspecKeywordText.toStdString(), datasetName, values );
    }

    return true;
}
