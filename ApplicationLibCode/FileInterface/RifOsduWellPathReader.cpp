/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024  Equinor ASA
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

#include "RifOsduWellPathReader.h"

#undef signals
#include <arrow/array/array_primitive.h>
#include <arrow/csv/api.h>
#include <arrow/io/api.h>
#include <arrow/scalar.h>
#include <arrow/util/cancel.h>
#include <parquet/arrow/reader.h>
#define signals Q_SIGNALS

#include "RiaLogging.h"
#include "RiaTextStringTools.h"

#include "RifArrowTools.h"
#include "RifAsciiDataParseOptions.h"
#include "RifByteArrayArrowRandomAccessFile.h"
#include "RifCsvUserDataParser.h"

#include "Well/RigWellPath.h"

#include "cvfObject.h"
#include "cvfVector3.h"

#include <QFileInfo>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<cvf::ref<RigWellPath>, QString> RifOsduWellPathReader::parseCsv( const QString& content )
{
    QString                        errorMessage;
    RifCsvUserDataPastedTextParser parser( content );

    RifAsciiDataParseOptions parseOptions;
    parseOptions.cellSeparator    = ",";
    parseOptions.decimalSeparator = ".";

    std::vector<std::pair<QString, std::vector<double>>> readValues;

    auto parseResult = parser.parse( parseOptions );
    if ( parseResult )
    {
        for ( auto s : parser.tableData().columnInfos() )
        {
            if ( s.dataType != Column::NUMERIC ) continue;

            QString             columnName = QString::fromStdString( s.columnName() );
            bool                isNumber   = false;
            auto                value      = columnName.toDouble( &isNumber );
            std::vector<double> values     = s.values;
            if ( isNumber )
            {
                values.insert( values.begin(), value );
            }
            readValues.push_back( { columnName, values } );
        }
    }

    const int MD_INDEX  = 0;
    const int TVD_INDEX = 1;
    const int X_INDEX   = 4;
    const int Y_INDEX   = 5;

    if ( readValues.size() == 10 )
    {
        const size_t firstSize = readValues[MD_INDEX].second.size();
        if ( ( firstSize == readValues[TVD_INDEX].second.size() ) && ( firstSize == readValues[X_INDEX].second.size() ) &&
             ( firstSize == readValues[Y_INDEX].second.size() ) )
        {
            std::vector<cvf::Vec3d> wellPathPoints;
            std::vector<double>     measuredDepths;

            for ( size_t i = 0; i < firstSize; i++ )
            {
                cvf::Vec3d point( readValues[X_INDEX].second[i], readValues[Y_INDEX].second[i], -readValues[TVD_INDEX].second[i] );
                double     md = readValues[MD_INDEX].second[i];

                wellPathPoints.push_back( point );
                measuredDepths.push_back( md );
            }

            return { new RigWellPath( wellPathPoints, measuredDepths ), "" };
        }
    }

    return { nullptr, "Oh no!" };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<cvf::ref<RigWellPath>, QString> RifOsduWellPathReader::readWellPathData( const QByteArray& content, double datumElevation )
{
    arrow::MemoryPool* pool = arrow::default_memory_pool();

    std::shared_ptr<arrow::io::RandomAccessFile> input = std::make_shared<RifByteArrayArrowRandomAccessFile>( content );

    // Open Parquet file reader
    std::unique_ptr<parquet::arrow::FileReader> arrow_reader;
    if ( !parquet::arrow::OpenFile( input, pool, &arrow_reader ).ok() )
    {
        return { nullptr, "Unable to read parquet data." };
    }

    // Read entire file as a single Arrow table
    std::shared_ptr<arrow::Table> table;
    if ( !arrow_reader->ReadTable( &table ).ok() )
    {
        return { nullptr, "Unable to read parquet table." };
    }

    const std::string MD  = "MD";
    const std::string TVD = "TVD";
    const std::string X   = "X";
    const std::string Y   = "Y";

    std::vector<std::string> columnNames = { MD, TVD, X, Y };

    std::map<std::string, std::vector<double>> readValues;

    for ( std::string columnName : columnNames )
    {
        std::shared_ptr<arrow::ChunkedArray> column = table->GetColumnByName( columnName );

        if ( column->type()->id() == arrow::Type::DOUBLE )
        {
            std::vector<double> columnVector = RifArrowTools::chunkedArrayToVector<arrow::DoubleArray, double>( column );
            RiaLogging::debug( QString( "Column name: %1. Size: %2" ).arg( QString::fromStdString( columnName ) ).arg( columnVector.size() ) );
            readValues[columnName] = columnVector;
        }
    }

    const size_t firstSize = readValues[MD].size();
    if ( ( firstSize == readValues[TVD].size() ) && ( firstSize == readValues[X].size() ) && ( firstSize == readValues[Y].size() ) )
    {
        std::vector<cvf::Vec3d> wellPathPoints;
        std::vector<double>     measuredDepths;

        for ( size_t i = 0; i < firstSize; i++ )
        {
            cvf::Vec3d point( readValues[X][i], readValues[Y][i], -readValues[TVD][i] + datumElevation );
            double     md = readValues[MD][i];

            wellPathPoints.push_back( point );
            measuredDepths.push_back( md );
        }

        auto wellPath = cvf::make_ref<RigWellPath>( wellPathPoints, measuredDepths );
        wellPath->setDatumElevation( datumElevation );
        return { wellPath, "" };
    }

    return { nullptr, "" };
}
