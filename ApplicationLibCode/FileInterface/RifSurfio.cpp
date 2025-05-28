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
#include "RifFileTools.h"

#include "irap_import.h"

#include <fstream>

//--------------------------------------------------------------------------------------------------
///
/// https://github.com/equinor/xtgeo/blob/main/src/xtgeo/surface/_regsurf_import.py
/// https://xtgeo.readthedocs.io/en/latest/datamodels.html#description
/// https://github.com/equinor/surfio
///
//--------------------------------------------------------------------------------------------------
std::expected<std::pair<RigRegularSurfaceData, std::vector<float>>, std::string> RifSurfio::importSurfaceData( const std::string& filename )
{
    auto extension = RifFileTools::extensionLowerCase( filename );
    if ( extension != ".irap" && extension != ".gri" )
    {
        return std::unexpected( "File is not a valid IRAP or GRI file: " + filename );
    }

    // Check file permissions (best effort, may not be reliable on all platforms)
    auto isFileReadable = RifFileTools::isFileReadable( filename );
    if ( !isFileReadable.has_value() )
    {
        return std::unexpected( isFileReadable.error() );
    }

    // Try to open the file to confirm readability
    std::ifstream file( filename );
    if ( !file.is_open() )
    {
        return std::unexpected( "Could not open file: " + filename );
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
        for ( int row = 0; row < fileData.header.nrow; ++row )
        {
            for ( int col = 0; col < fileData.header.ncol; ++col )
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
