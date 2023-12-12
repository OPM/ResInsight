/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023-     Equinor ASA
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

#include "RifInpIncludeReader.h"

#include "RiaStdStringTools.h"

#include <limits>
#include <string_view>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifInpIncludeReader::RifInpIncludeReader()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifInpIncludeReader::~RifInpIncludeReader()
{
    if ( isOpen() ) close();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifInpIncludeReader::openFile( const std::string& fileName )
{
    m_stream.open( fileName );
    return m_stream.good();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifInpIncludeReader::isOpen() const
{
    return m_stream.is_open();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifInpIncludeReader::close()
{
    m_stream.close();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifInpIncludeReader::readData( int columnIndex, const std::map<int, std::string>& parts, std::map<int, std::vector<double>>& data )
{
    std::map<std::string, int> nameToPartId;
    for ( auto p : parts )
    {
        nameToPartId[p.second] = p.first;
    }

    std::string line;

    while ( true )
    {
        std::getline( m_stream, line );

        if ( m_stream )
        {
            if ( line.starts_with( '*' ) ) continue;
            if ( line.starts_with( ',' ) ) continue;

            // is the requested column present?
            auto columns = RiaStdStringTools::splitString( line, ',' );
            if ( columnIndex >= columns.size() ) continue;

            // split part/set/node/element in first column
            auto partNode = RiaStdStringTools::splitString( columns[0], '.' );
            if ( partNode.size() != 3 ) continue;
            auto& partName       = partNode[0];
            int   nodeOrElmIndex = RiaStdStringTools::toInt( partNode[2] ) - 1;
            if ( nodeOrElmIndex < 0 ) continue;

            // is it a valid part name?
            if ( nameToPartId.count( partName ) == 0 ) continue;
            int partId = nameToPartId[partName];

            // is the index as expected?
            if ( nodeOrElmIndex >= (int)data[partId].size() ) continue;

            // is it a valid value?
            double value = std::numeric_limits<double>::infinity();
            RiaStdStringTools::toDouble( RiaStdStringTools::trimString( columns[columnIndex] ), value );

            data[partId][nodeOrElmIndex] = value;
        }

        if ( m_stream.eof() ) break;
    }
}
