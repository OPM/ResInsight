/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RifFileTools.h"

#include <algorithm>
#include <filesystem>

namespace RifFileTools
{
namespace fs = std::filesystem;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<bool, std::string> isFileReadable( const std::string& filename )
{
    // Check if file exists and is a regular file
    if ( !fs::exists( filename ) || !fs::is_regular_file( filename ) )
    {
        return std::unexpected( "File does not exist or is not a regular file: " + filename );
    }

    auto p          = fs::status( filename ).permissions();
    auto isReadable = ( p & fs::perms::owner_read ) != fs::perms::none || ( p & fs::perms::group_read ) != fs::perms::none ||
                      ( p & fs::perms::others_read ) != fs::perms::none;

    if ( !isReadable )
    {
        return std::unexpected( "File is not readable: " + filename );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string extensionLowerCase( const std::string& filename )
{
    if ( filename.empty() ) return "";

    std::string extension = fs::path( filename ).extension().string();
    std::transform( extension.begin(), extension.end(), extension.begin(), ::tolower );
    return extension;
}
}; // namespace RifFileTools
