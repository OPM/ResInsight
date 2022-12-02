/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022  Equinor ASA
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

#include "RifSeismicZGYReader.h"

#include <openzgy/openzgy.h>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSeismicZGYReader::RifSeismicZGYReader( QString filename )
    : m_filename( filename )
    , m_reader( nullptr )
{
    m_reader = OpenZGY::IZgyReader::open( filename.toStdString() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSeismicZGYReader::~RifSeismicZGYReader()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifSeismicZGYReader::Open()
{
    if ( m_reader != nullptr ) return false;

    try
    {
        m_reader = OpenZGY::IZgyReader::open( m_filename.toStdString() );
    }
    catch ( const std::exception& )
    {
        m_reader = nullptr;
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifSeismicZGYReader::Close()
{
    if ( m_reader == nullptr ) return;

    try
    {
        m_reader->close();
    }
    catch ( const std::exception& )
    {
    }

    m_reader = nullptr;

    return;
}
