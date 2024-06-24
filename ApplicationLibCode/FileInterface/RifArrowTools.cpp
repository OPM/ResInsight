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

#include "cafAssert.h"

#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RifArrowTools::convertChunkedArrayToStdVector( const std::shared_ptr<arrow::ChunkedArray>& column )
{
    auto convertChunkToVector = []( const std::shared_ptr<arrow::Array>& array ) -> std::vector<double>
    {
        std::vector<double> result;

        auto double_array = std::static_pointer_cast<arrow::DoubleArray>( array );
        result.resize( double_array->length() );
        for ( int64_t i = 0; i < double_array->length(); ++i )
        {
            result[i] = double_array->Value( i );
        }

        return result;
    };

    CAF_ASSERT( column->type()->id() == arrow::Type::DOUBLE );

    std::vector<double> result;

    // Iterate over each chunk in the column
    for ( int i = 0; i < column->num_chunks(); ++i )
    {
        std::shared_ptr<arrow::Array> chunk        = column->chunk( i );
        std::vector<double>           chunk_vector = convertChunkToVector( chunk );
        result.insert( result.end(), chunk_vector.begin(), chunk_vector.end() );
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<float> RifArrowTools::convertChunkedArrayToStdFloatVector( const std::shared_ptr<arrow::ChunkedArray>& column )
{
    auto convertChunkToFloatVector = []( const std::shared_ptr<arrow::Array>& array ) -> std::vector<float>
    {
        std::vector<float> result;

        auto arrowFloatArray = std::static_pointer_cast<arrow::FloatArray>( array );
        result.resize( arrowFloatArray->length() );
        for ( int64_t i = 0; i < arrowFloatArray->length(); ++i )
        {
            result[i] = arrowFloatArray->Value( i );
        }

        return result;
    };

    CAF_ASSERT( column->type()->id() == arrow::Type::FLOAT );

    std::vector<float> result;

    // Iterate over each chunk in the column
    for ( int i = 0; i < column->num_chunks(); ++i )
    {
        std::shared_ptr<arrow::Array> chunk        = column->chunk( i );
        std::vector<float>            chunk_vector = convertChunkToFloatVector( chunk );
        result.insert( result.end(), chunk_vector.begin(), chunk_vector.end() );
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifArrowTools::readSummaryData_debug( const QByteArray& contents )
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
            std::vector<double> columnVector = RifArrowTools::convertChunkedArrayToStdVector( column );
            columnVectors.push_back( columnVector );
        }
        else if ( column->type()->id() == arrow::Type::FLOAT )
        {
            auto                floatVector = RifArrowTools::convertChunkedArrayToStdFloatVector( column );
            std::vector<double> columnVector( floatVector.begin(), floatVector.end() );
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
