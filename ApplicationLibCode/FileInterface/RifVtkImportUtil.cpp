/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "RifVtkImportUtil.h"

#include "RiaStdStringTools.h"

#include <filesystem>
#include <spanstream>
#include <sstream>
#include <string>
#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RifVtkImportUtil::readPoints( const pugi::xml_node& piece )
{
    auto points = piece.child( "Points" );
    if ( !points ) return {};

    auto coords = points.child( "DataArray" );
    if ( !coords || std::string( coords.attribute( "Name" ).value() ) != "Coordinates" ) return {};

    std::string_view        text     = coords.text().get();
    std::vector<cvf::Vec3d> vertices = parseVec3ds( text );

    // Update to internal z direction
    for ( cvf::Vec3d& v : vertices )
        v.z() = -v.z();

    return vertices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3f> RifVtkImportUtil::readDisplacements( const pugi::xml_node& piece )
{
    auto pointData = piece.child( "PointData" );
    if ( !pointData ) return {};

    auto dataArray = pointData.child( "DataArray" );
    if ( !dataArray || std::string( dataArray.attribute( "Name" ).value() ) != "disp" ) return {};

    std::string_view text = dataArray.text().get();
    return parseVec3fs( text );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<unsigned> RifVtkImportUtil::readConnectivity( const pugi::xml_node& piece )
{
    auto cells = piece.child( "Cells" );
    if ( !cells ) return {};

    std::vector<unsigned> connectivity;
    auto                  connectivityArray = cells.child( "DataArray" );

    while ( connectivityArray )
    {
        if ( std::string( connectivityArray.attribute( "Name" ).value() ) == "connectivity" )
        {
            std::string        connectivityText = connectivityArray.text().get();
            std::istringstream iss( connectivityText );

            unsigned index;
            while ( iss >> index )
            {
                connectivity.push_back( index );
            }
            break;
        }
        connectivityArray = connectivityArray.next_sibling( "DataArray" );
    }

    return connectivity;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<float>> RifVtkImportUtil::readProperties( const pugi::xml_node& piece )
{
    auto cellData = piece.child( "CellData" );
    if ( !cellData ) return {};

    std::map<std::string, std::vector<float>> properties;

    for ( const auto& dataArray : cellData.children( "DataArray" ) )
    {
        const char* name = dataArray.attribute( "Name" ).value();
        if ( name && *name )
        {
            std::vector<float> values;
            std::istringstream iss( dataArray.text().get() );

            std::string token;
            while ( iss >> token )
            {
                if ( token == "nan" || token == "NaN" || token == "NAN" )
                {
                    values.push_back( std::numeric_limits<float>::quiet_NaN() );
                }
                else
                {
                    double value = 0.0;
                    bool   isOk  = RiaStdStringTools::toDouble( token, value );
                    if ( isOk )
                        values.push_back( static_cast<float>( value ) );
                    else
                        return {};
                }
            }

            if ( !values.empty() )
            {
                properties[name] = std::move( values );
            }
        }
    }

    return properties;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifVtkImportUtil::PvdDataset> RifVtkImportUtil::parsePvdDatasets( const std::filesystem::path& filepath )
{
    pugi::xml_document     doc;
    pugi::xml_parse_result result = doc.load_file( filepath.string().c_str() );
    if ( !result ) return {};

    auto root = doc.child( "VTKFile" );
    if ( !root ) return {};

    auto collection = root.child( "Collection" );
    if ( !collection ) return {};

    std::filesystem::path baseDir = filepath.parent_path();

    std::vector<PvdDataset> datasets;

    for ( const auto& datasetElem : collection.children( "DataSet" ) )
    {
        const char* file        = datasetElem.attribute( "file" ).value();
        const char* timestepStr = datasetElem.attribute( "timestep" ).value();

        if ( file && timestepStr )
        {
            double                timestep = std::stod( timestepStr );
            std::filesystem::path fullPath = std::filesystem::absolute( baseDir / file );

            datasets.push_back( { timestep, fullPath } );
        }
    }

    return datasets;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RifVtkImportUtil::parseVec3ds( std::string_view text )
{
    std::ispanstream iss( text );

    std::vector<cvf::Vec3d> vecs;

    std::string xStr, yStr, zStr;
    while ( iss >> xStr >> yStr >> zStr )
    {
        double x, y, z;
        if ( !RiaStdStringTools::toDouble( xStr, x ) || !RiaStdStringTools::toDouble( yStr, y ) || !RiaStdStringTools::toDouble( zStr, z ) )
        {
            return {};
        }

        vecs.emplace_back( x, y, z );
    }

    return vecs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3f> RifVtkImportUtil::parseVec3fs( std::string_view text )
{
    std::ispanstream iss( text );

    std::vector<cvf::Vec3f> vecs;

    std::string xStr, yStr, zStr;
    while ( iss >> xStr >> yStr >> zStr )
    {
        double x, y, z;
        if ( !RiaStdStringTools::toDouble( xStr, x ) || !RiaStdStringTools::toDouble( yStr, y ) || !RiaStdStringTools::toDouble( zStr, z ) )
        {
            return {};
        }

        vecs.emplace_back( static_cast<float>( x ), static_cast<float>( y ), static_cast<float>( z ) );
    }

    return vecs;
}
