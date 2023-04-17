
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

#include "RifOpenVDSReader.h"

#include <zgyaccess/seismicslice.h>

#include "cvfBoundingBox.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifOpenVDSReader::RifOpenVDSReader()
    : m_filename( "" )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifOpenVDSReader::~RifOpenVDSReader()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpenVDSReader::open( QString filename )
{
    if ( isOpen() ) close();

    m_filename = filename;

    try
    {
        // m_reader = std::make_unique<ZGYAccess::ZGYReader>();
        // if ( !m_reader->open( filename.toStdString() ) )
        //{
        //     m_reader.reset();
        //     return false;
        // }
    }
    catch ( const std::exception& err )
    {
        // m_reader.reset();
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpenVDSReader::isOpen() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifOpenVDSReader::isValid()
{
    if ( !isOpen() ) return false;

    bool valid = ( zStep() > 0.0 ) && ( inlineMinMaxStep()[2] > 0 ) && ( xlineMinMaxStep()[2] > 0 );
    return valid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOpenVDSReader::close()
{
    if ( !isOpen() ) return;

    try
    {
        // m_reader->close();
    }
    catch ( const std::exception& )
    {
    }

    // m_reader.reset();

    return;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, QString>> RifOpenVDSReader::metaData()
{
    std::vector<std::pair<QString, QString>> retValues;

    if ( !isOpen() ) return retValues;

    // auto stats = m_reader->metaData();

    // for ( auto& [name, val] : stats )
    //{
    //     retValues.push_back( std::make_pair( QString::fromStdString( name ), QString::fromStdString( val ) ) );
    // }

    return retValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RifOpenVDSReader::boundingBox()
{
    cvf::BoundingBox retBox;

    if ( isOpen() )
    {
        // auto [zmin, zmax] = m_reader->zRange();

        // auto outline = m_reader->seismicWorldOutline();

        // auto corners = outline.points();
        // for ( auto p : corners )
        //{
        //     retBox.add( cvf::Vec3d( p.x(), p.y(), -zmin ) );
        //     retBox.add( cvf::Vec3d( p.x(), p.y(), -zmax ) );
        // }
    }

    return retBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifOpenVDSReader::histogramData( std::vector<double>& xvals, std::vector<double>& yvals )
{
    if ( !isOpen() ) return;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RifOpenVDSReader::dataRange()
{
    if ( !isOpen() ) return std::make_pair( 0.0, 0.0 );

    return std::make_pair( 0.0, 0.0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RifOpenVDSReader::worldCorners()
{
    if ( !isOpen() ) return {};

    // auto [zmin, zmax] = m_reader->zRange();
    // auto outline      = m_reader->seismicWorldOutline();

    std::vector<cvf::Vec3d> retval;

    // for ( auto p : outline.points() )
    //{
    //     retval.push_back( cvf::Vec3d( p.x(), p.y(), -zmin ) );
    //     retval.push_back( cvf::Vec3d( p.x(), p.y(), -zmax ) );
    // }

    return retval;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RifOpenVDSReader::zStep()
{
    if ( !isOpen() ) return 0.0;

    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifOpenVDSReader::zSize()
{
    if ( !isOpen() ) return 0;

    return 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3i RifOpenVDSReader::inlineMinMaxStep()
{
    if ( !isOpen() ) return { 0, 0, 0 };
    return { 0, 0, 0 };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3i RifOpenVDSReader::xlineMinMaxStep()
{
    if ( !isOpen() ) return { 0, 0, 0 };
    return { 0, 0, 0 };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RifOpenVDSReader::convertToWorldCoords( int iLine, int xLine, double depth )
{
    if ( !isOpen() ) return { 0, 0, 0 };
    return { 0, 0, 0 };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<int, int> RifOpenVDSReader::convertToInlineXline( double worldx, double worldy )
{
    if ( !isOpen() ) return { 0, 0 };

    return { 0, 0 };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ZGYAccess::SeismicSliceData>
    RifOpenVDSReader::slice( RiaDefines::SeismicSliceDirection direction, int sliceIndex, int zStartIndex, int zSize )
{
    if ( isOpen() )
    {
        // switch ( direction )
        //{
        //     case RiaDefines::SeismicSliceDirection::INLINE:
        //         if ( zStartIndex < 0 ) return m_reader->inlineSlice( sliceIndex );
        //         return m_reader->inlineSlice( sliceIndex, zStartIndex, zSize );
        //     case RiaDefines::SeismicSliceDirection::XLINE:
        //         if ( zStartIndex < 0 ) return m_reader->xlineSlice( sliceIndex );
        //         return m_reader->xlineSlice( sliceIndex, zStartIndex, zSize );
        //     case RiaDefines::SeismicSliceDirection::DEPTH:
        //         return m_reader->zSlice( sliceIndex );
        //     default:
        //         break;
        // }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<ZGYAccess::SeismicSliceData> RifOpenVDSReader::trace( int inlineIndex, int xlineIndex, int zStartIndex, int zSize )
{
    if ( isOpen() )
    {
        // if ( zStartIndex < 0 ) return m_reader->zTrace( inlineIndex, xlineIndex );
        // return m_reader->zTrace( inlineIndex, xlineIndex, zStartIndex, zSize );
    }

    return nullptr;
}
