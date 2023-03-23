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

#include <zgyaccess/seismicslice.h>
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
        m_reader = std::make_unique<ZGYAccess::ZGYReader>();
        if ( !m_reader->open( filename.toStdString() ) )
        {
            m_reader.reset();
            return false;
        }
    }
    catch ( const std::exception& err )
    {
        m_reader.reset();
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifSeismicZGYReader::isOpen() const
{
    return m_reader.get() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifSeismicZGYReader::close()
{
    if ( !isOpen() ) return;

    try
    {
        m_reader->close();
    }
    catch ( const std::exception& )
    {
    }

    m_reader.reset();

    return;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, QString>> RifSeismicZGYReader::metaData()
{
    std::vector<std::pair<QString, QString>> retValues;

    if ( !isOpen() ) return retValues;

    auto stats = m_reader->metaData();

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
        auto [zmin, zmax] = m_reader->zRange();

        auto outline = m_reader->seismicWorldOutline();

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

    return m_reader->dataRange();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RifSeismicZGYReader::worldCorners()
{
    if ( !isOpen() ) return {};

    auto [zmin, zmax] = m_reader->zRange();
    auto outline      = m_reader->seismicWorldOutline();

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
double RifSeismicZGYReader::zStep()
{
    if ( !isOpen() ) return 0.0;

    return m_reader->zStep();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifSeismicZGYReader::zSize()
{
    if ( !isOpen() ) return 0;

    return m_reader->zSize();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3i RifSeismicZGYReader::inlineMinMaxStep()
{
    if ( !isOpen() ) return { 0, 0, 0 };

    auto [minVal, maxVal] = m_reader->inlineRange();
    int step              = m_reader->inlineStep();

    return { minVal, maxVal, step };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3i RifSeismicZGYReader::xMinMaxStep()
{
    if ( !isOpen() ) return { 0, 0, 0 };

    auto [minVal, maxVal] = m_reader->xlineRange();
    int step              = m_reader->xlineStep();

    return { minVal, maxVal, step };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RifSeismicZGYReader::convertToWorldCoords( int iLine, int xLine, double depth )
{
    if ( !isOpen() ) return { 0, 0, 0 };

    auto [x, y] = m_reader->toWorldCoordinate( iLine, xLine );

    return cvf::Vec3d( x, y, depth );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<int, int> RifSeismicZGYReader::convertToInlineXline( double worldx, double worldy )
{
    if ( !isOpen() ) return { 0, 0 };

    return m_reader->toInlineXline( worldx, worldy );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ZGYAccess::SeismicSliceData>
    RifSeismicZGYReader::slice( RiaDefines::SeismicSliceDirection direction, int sliceIndex, int zStartIndex, int zSize )
{
    if ( isOpen() )
    {
        switch ( direction )
        {
            case RiaDefines::SeismicSliceDirection::INLINE:
                if ( zStartIndex < 0 ) return m_reader->inlineSlice( sliceIndex );
                return m_reader->inlineSlice( sliceIndex, zStartIndex, zSize );
            case RiaDefines::SeismicSliceDirection::XLINE:
                if ( zStartIndex < 0 ) return m_reader->xlineSlice( sliceIndex );
                return m_reader->xlineSlice( sliceIndex, zStartIndex, zSize );
            case RiaDefines::SeismicSliceDirection::DEPTH:
                return m_reader->zSlice( sliceIndex );
            default:
                break;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ZGYAccess::SeismicSliceData> RifSeismicZGYReader::trace( int inlineIndex, int xlineIndex, int zStartIndex, int zSize )
{
    if ( isOpen() )
    {
        if ( zStartIndex < 0 ) return m_reader->zTrace( inlineIndex, xlineIndex );
        return m_reader->zTrace( inlineIndex, xlineIndex, zStartIndex, zSize );
    }

    return nullptr;
}
