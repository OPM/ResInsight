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

#include <zgyaccess/zgyreader.h>

#include "cvfBoundingBox.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSeismicZGYReader::RifSeismicZGYReader( QString filename )
    : m_filename( filename )
    , m_reader( nullptr )
{
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
        m_reader = std::make_shared<ZGYAccess::ZGYReader>();
        m_reader->Open( m_filename.toStdString() );
    }
    catch ( const std::exception& err )
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
        m_reader->Close();
    }
    catch ( const std::exception& )
    {
    }

    m_reader = nullptr;

    return;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, QString>> RifSeismicZGYReader::metaData()
{
    std::vector<std::pair<QString, QString>> retValues;

    if ( m_reader == nullptr ) return retValues;

    auto stats = m_reader->MetaData();

    for ( auto& [name, val] : stats )
    {
        retValues.push_back( std::make_pair( QString::fromStdString( name ), QString::fromStdString( val ) ) );
    }

    return retValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RifSeismicZGYReader::boundingBox()
{
    cvf::BoundingBox retBox;

    if ( m_reader != nullptr )
    {
        auto [zmin, zmax] = m_reader->ZRange();

        auto corners = m_reader->WorldCorners();
        for ( auto [x, y] : corners )
        {
            retBox.add( cvf::Vec3d( x, y, zmin ) );
            retBox.add( cvf::Vec3d( x, y, zmax ) );
        }
    }

    return retBox;
}
