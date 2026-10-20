/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Equinor ASA
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

#include "RifFormationNamesReader.h"

#include "RifFormationNamesFileParser.h"

#include "RiaLogging.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace
{
// Typical FMU location, relative to the folder of the grid case file (e.g. realization-0/iter-0/eclipse/model):
//   ../../rms/output/zone/layer_zone_table.txt
//   ../../rms/output/zone/stratigraphy.json
// stratigraphy.json is read from the same folder as the layer_zone_table.txt that is imported.
const QString layerZoneTableFileName = "layer_zone_table.txt";
const QString stratigraphyFileName   = "stratigraphy.json";
const QString zoneRangesYamlBaseName = "zonation_ecl_map";
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<RigFormationNames, QString> RifFormationNamesReader::readFormationNamesFile( const QString& fileName )
{
    QFileInfo fileInfo( fileName );

    const QString suffix = fileInfo.suffix().toLower();

    if ( ( suffix == "yml" || suffix == "yaml" ) && fileInfo.completeBaseName() != zoneRangesYamlBaseName )
    {
        // Other yaml files are ignored without reporting an error
        return RigFormationNames();
    }

    if ( fileInfo.fileName() == layerZoneTableFileName || suffix == "yml" || suffix == "yaml" )
    {
        std::map<QString, cvf::Color3f> colors;

        const QString colorFile = fileInfo.dir().absoluteFilePath( stratigraphyFileName );
        if ( QFile::exists( colorFile ) )
        {
            if ( auto result = RifFormationNamesFileParser::readStratigraphyColors( colorFile ); result )
            {
                colors = std::move( *result );
            }
            else
            {
                RiaLogging::warning( "Ignoring zone colors: " + result.error().toStdString() );
            }
        }

        if ( suffix == "yml" || suffix == "yaml" )
        {
            return RifFormationNamesFileParser::readYamlFormationNameFile( fileName, colors );
        }
        return RifFormationNamesFileParser::readFmuFormationNameFile( fileName, colors );
    }
    else
    {
        return RifFormationNamesFileParser::readLyrFormationNameFile( fileName );
    }
}
