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

#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "cvfVector3.h"

#include "pugixml.hpp"

//==================================================================================================
///
//==================================================================================================
class RifVtkImportUtil
{
public:
    struct PvdDataset
    {
        double                timestep;
        std::filesystem::path filepath;
    };

    static std::vector<PvdDataset> parsePvdDatasets( const std::filesystem::path& filepath );

    static std::vector<cvf::Vec3d>                   readPoints( const pugi::xml_node& piece );
    static std::vector<cvf::Vec3f>                   readDisplacements( const pugi::xml_node& piece );
    static std::vector<unsigned>                     readConnectivity( const pugi::xml_node& piece );
    static std::map<std::string, std::vector<float>> readProperties( const pugi::xml_node& piece );

private:
    static std::vector<cvf::Vec3d> parseVec3ds( std::string_view text );
    static std::vector<cvf::Vec3f> parseVec3fs( std::string_view text );
};
