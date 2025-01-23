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

#include "RifVtkSurfaceImporter.h"

#include "../../Fwk/VizFwk/LibIo/cvfTinyXmlFused.hpp"
#include "RigGocadData.h"

#include <filesystem>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace RifVtkSurfaceImporter
{
using namespace cvf_tinyXML;

bool importFromFile( std::string filename, RigGocadData* gocadData )
{
    TiXmlDocument doc;
    if ( !doc.LoadFile( filename.c_str() ) )
    {
        return false;
    }

    return importFromXmlDoc( doc, gocadData );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool importFromPvdFile( const std::string& filename, RigGocadData* gocadData )
{
    auto datasets = parsePvdDatasets( filename );

    if ( datasets.empty() ) return false;

    // Sort and import the most recent dataset
    std::sort( datasets.begin(), datasets.end(), []( const PvdDataset& a, const PvdDataset& b ) { return a.timestep < b.timestep; } );

    return importDataset( datasets.back(), gocadData );
}

bool importFromXmlDoc( const TiXmlDocument& doc, RigGocadData* gocadData )
{
    auto* root = doc.FirstChildElement( "VTKFile" );
    if ( !root ) return false;

    auto* grid = root->FirstChildElement( "UnstructuredGrid" );
    if ( !grid ) return false;

    auto* piece = grid->FirstChildElement( "Piece" );
    if ( !piece ) return false;

    // Read points
    std::vector<cvf::Vec3d> vertices;
    if ( !readPoints( piece, vertices ) )
    {
        return false;
    }

    // Read connectivity
    std::vector<unsigned> connectivity;
    if ( !readConnectivity( piece, connectivity ) )
    {
        return false;
    }

    // Avoid shared nodes
    std::vector<cvf::Vec3d> nonSharedVertices;
    std::vector<unsigned>   nonSharedConnectivity;

    const auto numTriangles = connectivity.size() / 3;
    for ( size_t triangleIdx = 0; triangleIdx < numTriangles; triangleIdx++ )
    {
        nonSharedVertices.push_back( vertices[connectivity[3 * triangleIdx + 0]] );
        nonSharedVertices.push_back( vertices[connectivity[3 * triangleIdx + 1]] );
        nonSharedVertices.push_back( vertices[connectivity[3 * triangleIdx + 2]] );

        nonSharedConnectivity.push_back( static_cast<unsigned int>( 3 * triangleIdx + 0 ) );
        nonSharedConnectivity.push_back( static_cast<unsigned int>( 3 * triangleIdx + 1 ) );
        nonSharedConnectivity.push_back( static_cast<unsigned int>( 3 * triangleIdx + 2 ) );
    }

    // Set geometry data
    gocadData->setGeometryData( nonSharedVertices, nonSharedConnectivity );

    // Read properties
    std::vector<std::string>        propertyNamesOnFile;
    std::vector<std::vector<float>> propertyValuesOnFile;

    readProperties( piece, propertyNamesOnFile, propertyValuesOnFile );

    if ( propertyNamesOnFile.size() == propertyValuesOnFile.size() )
    {
        for ( size_t i = 0; i < propertyValuesOnFile.size(); i++ )
        {
            // These values are per element, so we need to duplicate them for each node
            auto values = propertyValuesOnFile[i];
            if ( values.size() * 3 == nonSharedVertices.size() )
            {
                std::vector<float> valuesForEachNode;
                for ( auto value : values )
                {
                    valuesForEachNode.push_back( value );
                    valuesForEachNode.push_back( value );
                    valuesForEachNode.push_back( value );
                }

                gocadData->addPropertyData( QString::fromStdString( propertyNamesOnFile[i] ), valuesForEachNode );
            }
        }
    }

    return true;
}

bool readPoints( const TiXmlElement* piece, std::vector<cvf::Vec3d>& vertices )
{
    auto* points = piece->FirstChildElement( "Points" );
    if ( !points ) return false;

    auto* coords = points->FirstChildElement( "DataArray" );
    if ( !coords || strcmp( coords->Attribute( "Name" ), "Coordinates" ) != 0 ) return false;

    std::string        coordsText = coords->GetText();
    std::istringstream iss( coordsText );

    double x, y, z;
    while ( iss >> x >> y >> z )
    {
        vertices.push_back( cvf::Vec3d( x, y, -z ) );
    }

    return !vertices.empty();
}

bool readConnectivity( const TiXmlElement* piece, std::vector<unsigned>& connectivity )
{
    auto* cells = piece->FirstChildElement( "Cells" );
    if ( !cells ) return false;

    auto* connectivityArray = cells->FirstChildElement( "DataArray" );
    while ( connectivityArray )
    {
        if ( strcmp( connectivityArray->Attribute( "Name" ), "connectivity" ) == 0 )
        {
            std::string        connectivityText = connectivityArray->GetText();
            std::istringstream iss( connectivityText );

            unsigned index;
            while ( iss >> index )
            {
                connectivity.push_back( index );
            }
            break;
        }
        connectivityArray = connectivityArray->NextSiblingElement( "DataArray" );
    }

    return !connectivity.empty();
}

void readProperties( const TiXmlElement* piece, std::vector<std::string>& propertyNames, std::vector<std::vector<float>>& propertyValues )
{
    auto* cellData = piece->FirstChildElement( "CellData" );
    if ( !cellData ) return;

    auto* dataArray = cellData->FirstChildElement( "DataArray" );
    while ( dataArray )
    {
        const char* name = dataArray->Attribute( "Name" );
        if ( name )
        {
            std::vector<float> values;
            std::string        valuesText = dataArray->GetText();
            std::istringstream iss( valuesText );

            float value;
            while ( iss >> value )
            {
                values.push_back( value );
            }

            if ( !values.empty() )
            {
                propertyNames.push_back( name );
                propertyValues.push_back( values );
            }
        }
        dataArray = dataArray->NextSiblingElement( "DataArray" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifVtkSurfaceImporter::PvdDataset> parsePvdDatasets( const std::string& filename )
{
    std::vector<PvdDataset> datasets;
    TiXmlDocument           doc;

    if ( !doc.LoadFile( filename.c_str() ) ) return datasets;

    auto* root = doc.FirstChildElement( "VTKFile" );
    if ( !root ) return datasets;

    auto* collection = root->FirstChildElement( "Collection" );
    if ( !collection ) return datasets;

    std::string baseDir = std::filesystem::path( filename ).parent_path().string();

    auto* datasetElem = collection->FirstChildElement( "DataSet" );
    while ( datasetElem )
    {
        const char* file        = datasetElem->Attribute( "file" );
        const char* timestepStr = datasetElem->Attribute( "timestep" );

        if ( file && timestepStr )
        {
            double      timestep = std::stod( timestepStr );
            std::string fullPath = std::filesystem::absolute( std::filesystem::path( baseDir ) / file ).string();

            datasets.push_back( { timestep, fullPath, {} } );
        }

        datasetElem = datasetElem->NextSiblingElement( "DataSet" );
    }

    return datasets;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool importDataset( const PvdDataset& dataset, RigGocadData* gocadData )
{
    TiXmlDocument doc;
    if ( !doc.LoadFile( dataset.filename.c_str() ) )
    {
        return false;
    }

    return importFromXmlDoc( doc, gocadData );
}

}; // namespace RifVtkSurfaceImporter
