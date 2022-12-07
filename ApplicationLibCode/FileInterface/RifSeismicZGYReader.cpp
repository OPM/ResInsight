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
    //    m_reader = OpenZGY::IZgyReader::open( filename.toStdString() );
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
        m_reader->close();
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

    auto stats = m_reader->filestats();

    retValues.push_back( std::make_pair( "ZGY version", QString::number( stats->fileVersion() ) ) );
    if ( stats->isCompressed() )
    {
        retValues.push_back( std::make_pair( "Compressed", "yes" ) );
        retValues.push_back( std::make_pair( "Compression factor", QString::number( stats->compressionFactor() ) ) );
    }
    retValues.push_back( std::make_pair( "File size", QString::number( stats->fileSize() ) ) );
    retValues.push_back( std::make_pair( "Header size", QString::number( stats->headerSize() ) ) );

    QString tmp;

    for ( auto& a : m_reader->brickcount() )
    {
        tmp.append( QString( "%1x%2x%3 " ).arg( a[0], a[1], a[2] ) );
    }
    retValues.push_back( std::make_pair( "Brick count", tmp ) );
    retValues.push_back( std::make_pair( "Levels of details", QString::number( m_reader->nlods() ) ) );

    auto& bricksize = m_reader->bricksize();
    retValues.push_back(
        std::make_pair( "Brick size", QString( "%1 x %2 x %3" ).arg( bricksize[0], bricksize[1], bricksize[2] ) ) );

    switch ( m_reader->datatype() )
    {
        default:
        case OpenZGY::SampleDataType::unknown:
            tmp = "Unknown";
            break;
        case OpenZGY::SampleDataType::int8:
            tmp = "Signed 8-bit";
            break;
        case OpenZGY::SampleDataType::int16:
            tmp = "Signed 16-bit";
            break;
        case OpenZGY::SampleDataType::float32:
            tmp = " 32-bit Floating point";
            break;
    }
    retValues.push_back( std::make_pair( "Native data type", tmp ) );

    auto& datarange = m_reader->datarange();
    retValues.push_back( std::make_pair( "Data range", QString( "%1 - %2" ).arg( datarange[0], datarange[1] ) ) );

    retValues.push_back( std::make_pair( "Depth unit", QString::fromStdString( m_reader->zunitname() ) ) );
    retValues.push_back( std::make_pair( "Depth offset", QString::number( m_reader->zstart() ) ) );
    retValues.push_back( std::make_pair( "Depth increment", QString::number( m_reader->zinc() ) ) );

    retValues.push_back( std::make_pair( "Horizontal unit", QString::fromStdString( m_reader->hunitname() ) ) );

    auto& annotstart = m_reader->annotstart();
    retValues.push_back( std::make_pair( "First inline", QString::number( annotstart[0] ) ) );
    retValues.push_back( std::make_pair( "First crossline", QString::number( annotstart[1] ) ) );

    auto& annotinc = m_reader->annotinc();
    retValues.push_back( std::make_pair( "Inline increment", QString::number( annotinc[0] ) ) );
    retValues.push_back( std::make_pair( "Crossline increment", QString::number( annotinc[1] ) ) );

    tmp = "";
    for ( auto& c : m_reader->corners() )
    {
        tmp = tmp + cornerToString( c ) + " ";
    }
    retValues.push_back( std::make_pair( "World coord. corners", tmp ) );

    tmp = "";
    for ( auto& c : m_reader->indexcorners() )
    {
        tmp = tmp + cornerToString( c ) + " ";
    }
    retValues.push_back( std::make_pair( "Brick index corners", tmp ) );

    tmp = "";
    for ( auto& c : m_reader->annotcorners() )
    {
        tmp = tmp + cornerToString( c ) + " ";
    }
    retValues.push_back( std::make_pair( "Inline/crossline corners", tmp ) );

    return retValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifSeismicZGYReader::cornerToString( std::array<double, 2> corner )
{
    return QString( "(%1, %2)" ).arg( corner[0], corner[1] );
}
