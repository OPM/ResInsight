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

#include "RifSurfio.h"

#include "irap_import.h"

#include <filesystem>
#include <fstream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<std::pair<RigRegularSurfaceData, std::vector<float>>, std::string> RifSurfio::importSurfaceData( const std::string& filename )
{
    namespace fs = std::filesystem;

    // Check if file exists and is a regular file
    if ( !fs::exists( filename ) || !fs::is_regular_file( filename ) )
    {
        return std::unexpected( "File does not exist or is not a regular file: " + filename );
    }

    // Check file permissions (best effort, may not be reliable on all platforms)
    fs::perms p = fs::status( filename ).permissions();
    if ( ( p & fs::perms::owner_read ) == fs::perms::none && ( p & fs::perms::group_read ) == fs::perms::none &&
         ( p & fs::perms::others_read ) == fs::perms::none )
    {
        return std::unexpected( "File is not readable (permission denied): " + filename );
    }

    // Try to open the file to confirm readability
    std::ifstream file( filename );
    if ( !file.is_open() )
    {
        return std::unexpected( "Could not open file: " + filename );
    }

    std::string extension = fs::path( filename ).extension().string();
    std::transform( extension.begin(), extension.end(), extension.begin(), ::tolower );
    if ( extension != ".irap" && extension != ".gri" )
    {
        return std::unexpected( "File is not a valid IRAP or GRI file: " + filename );
    }

    auto convertToRegularSurface =
        []( const surfio::irap::irap& fileData ) -> std::expected<std::pair<RigRegularSurfaceData, std::vector<float>>, std::string>
    {
        RigRegularSurfaceData surfaceData;
        surfaceData.nx         = fileData.header.ncol;
        surfaceData.ny         = fileData.header.nrow;
        surfaceData.originX    = fileData.header.xori;
        surfaceData.originY    = fileData.header.yori;
        surfaceData.incrementX = fileData.header.xinc;
        surfaceData.incrementY = fileData.header.yinc;
        surfaceData.rotation   = fileData.header.rot;

        // transpose the data to match the expected format
        std::vector<float> transposedValues( fileData.values.size() );
        for ( size_t row = 0; row < fileData.header.nrow; ++row )
        {
            for ( size_t col = 0; col < fileData.header.ncol; ++col )
            {
                transposedValues[row * fileData.header.ncol + col] = fileData.values[col * fileData.header.nrow + row];
            }
        }

        return std::make_pair( surfaceData, transposedValues );
    };

    if ( extension == ".gri" )
    {
        auto fileData = surfio::irap::from_binary_file( filename );

        return convertToRegularSurface( fileData );
    }
    else if ( extension == ".irap" )
    {
        auto fileData = surfio::irap::from_ascii_file( filename );

        return convertToRegularSurface( fileData );
    }
    else
    {
        return std::unexpected( "File is not a valid IRAP or GRI file: " + filename );
    }
}
