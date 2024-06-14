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

#include "cafAssert.h"

#include <vector>

// #include <arrow/array/array_primitive.h>
// #include <arrow/csv/api.h>
// #include <arrow/io/api.h>
// #include <arrow/scalar.h>
// #include <parquet/arrow/reader.h>

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
};
