/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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

#pragma once

#undef signals
#include <arrow/array/array_binary.h>
#include <arrow/array/array_primitive.h>

#define signals Q_SIGNALS

#include <limits>
#include <memory>
#include <vector>

#include <QByteArray>
#include <QString>

//==================================================================================================
//
//
//==================================================================================================
namespace RifArrowTools
{

// Template class used to handle most of the basic types. Conversiont to std::string requuires a specialization using chunk->GetString(j).
template <typename ArrowArrayType, typename CType>
std::vector<CType> chunkedArrayToVector( const std::shared_ptr<arrow::ChunkedArray>& chunkedArray )
{
    static_assert( std::is_base_of<arrow::Array, ArrowArrayType>::value, "ArrowArrayType must be derived from arrow::Array" );

    std::vector<CType> result;
    for ( int i = 0; i < chunkedArray->num_chunks(); ++i )
    {
        auto chunk = std::static_pointer_cast<ArrowArrayType>( chunkedArray->chunk( i ) );

        // Use auto here instead of CType to allow conversion between different types
        // Use raw_values() to get the raw data pointer for best performance
        const auto* data = chunk->raw_values();

        for ( int j = 0; j < chunk->length(); ++j )
        {
            if ( !chunk->IsNull( j ) )
            {
                result.push_back( data[j] );
            }
            else
            {
                result.push_back( std::numeric_limits<CType>::quiet_NaN() );
            }
        }
    }

    return result;
}

std::vector<std::string> chunkedArrayToStringVector( const std::shared_ptr<arrow::ChunkedArray>& chunkedArray );

QString readFirstRowsOfTable( const QByteArray& contents );

}; // namespace RifArrowTools
