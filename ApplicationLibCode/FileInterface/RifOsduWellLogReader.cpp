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

#include "RifOsduWellLogReader.h"

#include "RifByteArrayArrowRandomAccessFile.h"

#include "cafAssert.h"

#include <iostream>
#include <vector>

#include "RifArrowTools.h"

#include <arrow/array/array_primitive.h>
#include <arrow/csv/api.h>
#include <arrow/io/api.h>
#include <arrow/scalar.h>
#include <parquet/arrow/reader.h>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<cvf::ref<RigOsduWellLogData>, QString> RifOsduWellLogReader::readWellLogData( const QByteArray& contents )
{
    arrow::MemoryPool* pool = arrow::default_memory_pool();

    std::shared_ptr<arrow::io::RandomAccessFile> input = std::make_shared<RifByteArrayArrowRandomAccessFile>( contents );

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

    auto logData = cvf::make_ref<RigOsduWellLogData>();
    for ( std::string columnName : table->ColumnNames() )
    {
        std::shared_ptr<arrow::ChunkedArray> column = table->GetColumnByName( columnName );

        if ( column->type()->id() == arrow::Type::DOUBLE )
        {
            std::vector<double> columnVector = RifArrowTools::convertChunkedArrayToStdVector( column );
            logData->setValues( QString::fromStdString( columnName ), columnVector );
        }
    }

    logData->finalizeData();

    return { logData, "" };
}
