/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RifWellPathFormationsImporter.h"
#include "RifWellPathFormationReader.h"

#include "Riu3DMainWindowTools.h"
#include "RiuMessageDialog.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<RigWellPathFormations> RifWellPathFormationsImporter::readWellPathFormations( const QString& formationFilePath,
                                                                                            const QString& wellName )
{
    readAllWellPathFormations( formationFilePath );

    const auto& wellFormations = m_fileNameToWellPathFormationMap[formationFilePath];
    if ( auto it = wellFormations.find( wellName ); it != wellFormations.end() )
    {
        return it->second;
    }

    return std::nullopt;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<RigWellPathFormations> RifWellPathFormationsImporter::reloadWellPathFormations( const QString& formationFilePath,
                                                                                              const QString& wellName )
{
    m_fileNameToWellPathFormationMap.erase( formationFilePath );

    return readWellPathFormations( formationFilePath, wellName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, RigWellPathFormations> RifWellPathFormationsImporter::readWellPathFormationsFromPath( const QString& filePath )
{
    readAllWellPathFormations( filePath );

    return m_fileNameToWellPathFormationMap[filePath];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifWellPathFormationsImporter::reloadAllWellPathFormations()
{
    std::vector<QString> allFilePaths;
    for ( auto it = m_fileNameToWellPathFormationMap.begin(); it != m_fileNameToWellPathFormationMap.end(); it++ )
    {
        allFilePaths.push_back( it->first );
    }

    m_fileNameToWellPathFormationMap.clear();

    for ( const QString& filePath : allFilePaths )
    {
        readWellPathFormationsFromPath( filePath );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifWellPathFormationsImporter::readAllWellPathFormations( const QString& filePath )
{
    // If we have the file in the map, assume it is already read.
    if ( m_fileNameToWellPathFormationMap.find( filePath ) != m_fileNameToWellPathFormationMap.end() )
    {
        return;
    }

    auto wellFormations = RifWellPathFormationReader::readWellFormations( filePath );
    if ( !wellFormations )
    {
        RiuMessageDialog::showError( Riu3DMainWindowTools::mainWindowWidget(), "Import failure", wellFormations.error() );
    }

    m_fileNameToWellPathFormationMap[filePath] = wellFormations.value_or( RifWellPathFormationReader::WellFormations() );
}
