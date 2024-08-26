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

#include "RifArrowTools.h"

#include "RifByteArrayArrowRandomAccessFile.h"
#include "RifCsvDataTableFormatter.h"

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifArrowTools::readFirstRowsOfTable( const QByteArray& contents )
{
    arrow::MemoryPool* pool = arrow::default_memory_pool();

    std::shared_ptr<arrow::io::RandomAccessFile> input = std::make_shared<RifByteArrayArrowRandomAccessFile>( contents );

    // Open Parquet file reader
    std::unique_ptr<parquet::arrow::FileReader> arrow_reader;
    if ( !parquet::arrow::OpenFile( input, pool, &arrow_reader ).ok() )
    {
        return {};
    }

    // Read entire file as a single Arrow table
    std::shared_ptr<arrow::Table> table;
    if ( !arrow_reader->ReadTable( &table ).ok() )
    {
        return {};
    }

    QString                  tableText;
    QTextStream              stream( &tableText );
    RifCsvDataTableFormatter formatter( stream, ";" );

    std::vector<RifTextDataTableColumn> header;
    for ( std::string columnName : table->ColumnNames() )
    {
        header.push_back( RifTextDataTableColumn( QString::fromStdString( columnName ) ) );
    }

    formatter.header( header );

    std::vector<std::vector<double>> columnVectors;

    for ( std::string columnName : table->ColumnNames() )
    {
        std::shared_ptr<arrow::ChunkedArray> column = table->GetColumnByName( columnName );

        auto columnType = column->type()->id();

        if ( columnType == arrow::Type::DOUBLE )
        {
            std::vector<double> columnVector = RifArrowTools::chunkedArrayToVector<arrow::DoubleArray, double>( column );
            columnVectors.push_back( columnVector );
        }
        else if ( columnType == arrow::Type::FLOAT )
        {
            auto columnVector = RifArrowTools::chunkedArrayToVector<arrow::FloatArray, double>( column );
            columnVectors.push_back( columnVector );
        }
        else if ( columnType == arrow::Type::TIMESTAMP )
        {
            auto columnVector = RifArrowTools::chunkedArrayToVector<arrow::Int64Array, double>( column );
            columnVectors.push_back( columnVector );
        }
    }

    if ( columnVectors.empty() )
    {
        return {};
    }

    for ( int i = 0; i < std::min( 20, int( columnVectors[0].size() ) ); i++ )
    {
        for ( int j = 0; j < int( columnVectors.size() ); j++ )
        {
            formatter.add( columnVectors[j][i] );
        }
        formatter.rowCompleted();
    }

    formatter.tableCompleted();

    return tableText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RifArrowTools::chunkedArrayToStringVector( const std::shared_ptr<arrow::ChunkedArray>& chunkedArray )
{
    std::vector<std::string> result;

    for ( int i = 0; i < chunkedArray->num_chunks(); ++i )
    {
        auto chunk = std::static_pointer_cast<arrow::StringArray>( chunkedArray->chunk( i ) );
        for ( int j = 0; j < chunk->length(); ++j )
        {
            if ( !chunk->IsNull( j ) )
            {
                result.push_back( chunk->Value( j ).data() );
            }
        }
    }

    return result;
}
