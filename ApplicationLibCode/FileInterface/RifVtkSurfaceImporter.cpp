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

#include "RifVtkImportUtil.h"

#include "Surface/RigTriangleMeshData.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<RigTriangleMeshData> RifVtkSurfaceImporter::importFromFile( const std::filesystem::path& filepath )
{
    pugi::xml_document     doc;
    pugi::xml_parse_result result = doc.load_file( filepath.string().c_str() );
    if ( !result ) return nullptr;

    return importFromXmlDoc( doc );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<RigTriangleMeshData> RifVtkSurfaceImporter::importFromXmlDoc( const pugi::xml_document& doc )
{
    auto root = doc.child( "VTKFile" );
    if ( !root ) return nullptr;

    auto grid = root.child( "UnstructuredGrid" );
    if ( !grid ) return nullptr;

    auto piece = grid.child( "Piece" );
    if ( !piece ) return nullptr;

    // Read points
    std::vector<cvf::Vec3d> vertices = RifVtkImportUtil::readPoints( piece );
    if ( vertices.empty() ) return nullptr;

    // Read connectivity
    std::vector<unsigned> connectivity = RifVtkImportUtil::readConnectivity( piece );
    if ( connectivity.empty() ) return nullptr;

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
    auto triangleMeshData = std::make_unique<RigTriangleMeshData>();
    triangleMeshData->setGeometryData( nonSharedVertices, nonSharedConnectivity );

    // Read properties
    const std::map<std::string, std::vector<float>> properties = RifVtkImportUtil::readProperties( piece );
    for ( const auto& [name, values] : properties )
    {
        // These values are per element, so we need to duplicate them for each node
        if ( values.size() * 3 == nonSharedVertices.size() )
        {
            std::vector<float> valuesForEachNode;
            for ( auto value : values )
            {
                valuesForEachNode.push_back( value );
                valuesForEachNode.push_back( value );
                valuesForEachNode.push_back( value );
            }

            triangleMeshData->addPropertyData( QString::fromStdString( name ), valuesForEachNode );
        }
    }

    return triangleMeshData;
}
