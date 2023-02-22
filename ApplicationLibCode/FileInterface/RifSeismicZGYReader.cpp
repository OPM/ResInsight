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
RifSeismicZGYReader::RifSeismicZGYReader()
    : m_filename( "" )
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
bool RifSeismicZGYReader::open( QString filename )
{
    if ( isOpen() ) close();

    m_filename = filename;

    try
    {
        m_reader = std::make_shared<ZGYAccess::ZGYReader>();
        m_reader->Open( filename.toStdString() );
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
bool RifSeismicZGYReader::isOpen() const
{
    return m_reader != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifSeismicZGYReader::close()
{
    if ( !isOpen() ) return;

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

    if ( !isOpen() ) return retValues;

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

    if ( isOpen() )
    {
        auto [zmin, zmax] = m_reader->ZRange();

        auto outline = m_reader->seismicOutline();

        auto corners = outline.points();
        for ( auto p : corners )
        {
            retBox.add( cvf::Vec3d( p.x(), p.y(), -zmin ) );
            retBox.add( cvf::Vec3d( p.x(), p.y(), -zmax ) );
        }
    }

    return retBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifSeismicZGYReader::histogramData( std::vector<double>& xvals, std::vector<double>& yvals )
{
    if ( !isOpen() ) return;

    auto histdata = m_reader->histogram();

    xvals = histdata->Xvalues;
    yvals = histdata->Yvalues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RifSeismicZGYReader::dataRange()
{
    if ( !isOpen() ) return std::make_pair( 0.0, 0.0 );

    return m_reader->DataRange();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RifSeismicZGYReader::worldCorners()
{
    auto [zmin, zmax] = m_reader->ZRange();
    auto outline      = m_reader->seismicOutline();

    std::vector<cvf::Vec3d> retval;

    for ( auto p : outline.points() )
    {
        retval.push_back( cvf::Vec3d( p.x(), p.y(), -zmin ) );
        retval.push_back( cvf::Vec3d( p.x(), p.y(), -zmax ) );
    }

    return retval;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RifSeismicZGYReader::depthStep()
{
    if ( !isOpen() ) return 0.0;

    return m_reader->ZStep();
}
