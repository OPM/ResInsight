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

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "cvfVector3.h"

class RigTriangleMeshData;

namespace cvf_tinyXML
{
class TiXmlDocument;
class TiXmlElement;
} // namespace cvf_tinyXML

//==================================================================================================
///
//==================================================================================================
namespace RifVtkSurfaceImporter
{

struct PvdDataset
{
    double      timestep;
    std::string filename;
};

bool importFromFile( std::string filename, RigTriangleMeshData* triangleMeshData );

bool importFromPvdFile( const std::string& filename, RigTriangleMeshData* triangleMeshData );
bool importFromXmlDoc( const cvf_tinyXML::TiXmlDocument& doc, RigTriangleMeshData* triangleMeshData );

bool readPoints( const cvf_tinyXML::TiXmlElement* piece, std::vector<cvf::Vec3d>& vertices );
bool readConnectivity( const cvf_tinyXML::TiXmlElement* piece, std::vector<unsigned>& connectivity );
void readProperties( const cvf_tinyXML::TiXmlElement* piece,
                     std::vector<std::string>&        propertyNames,
                     std::vector<std::vector<float>>& propertyValues );

std::vector<PvdDataset> parsePvdDatasets( const std::string& filename );
bool                    importDataset( const PvdDataset& dataset, RigTriangleMeshData* triangleMeshData );

}; // namespace RifVtkSurfaceImporter
