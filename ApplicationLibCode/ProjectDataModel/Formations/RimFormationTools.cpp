/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 - Equinor ASA
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

#include "RimFormationTools.h"

#include "RiaLogging.h"

#include "Formations/RimFormationNames.h"
#include "Formations/RimFormationNamesCollection.h"

#include "cafUtils.h"

#include <QDir>
#include <QFileInfo>
#include <QStringList>

//--------------------------------------------------------------------------------------------------
///  look for formation file two levels up from the egrid file
//--------------------------------------------------------------------------------------------------
QStringList RimFormationTools::formationFoldersFromCaseFileName( const QString caseFileName )
{
    QStringList folders;

    QFileInfo fi( caseFileName );

    auto formationFolder = QDir( fi.dir().path() + "/../../" );
    auto rmsFolder       = QDir( fi.dir().path() + "/../../rms/output/zone/" );

    folders.push_back( rmsFolder.absolutePath() ); // rms folder is 1st pri.
    folders.push_back( formationFolder.absolutePath() );

    return folders;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFormationNames* RimFormationTools::loadFormationNamesFromFolder( const QStringList& folderNames )
{
    // Priority: .lyr (may contain colors), then zone range yaml files, then FMU layer zone table
    const std::vector<QStringList> filterPriority = { { "*.lyr" },
                                                      { "zonation_ecl_map.yml", "zonation_ecl_map.yaml", "zone_layers.yml", "zone_layers.yaml" },
                                                      { "layer_zone_table.txt" } };

    QStringList fileList;

    for ( auto& folder : folderNames )
    {
        for ( const QStringList& filters : filterPriority )
        {
            fileList = caf::Utils::getFilesInDirectory( folder, filters, true /*absolute filename*/ );
            if ( !fileList.isEmpty() ) break;
        }
        if ( !fileList.isEmpty() ) break;
    }

    if ( fileList.isEmpty() ) return nullptr;

    RimFormationNamesCollection*    fomNameColl    = new RimFormationNamesCollection();
    std::vector<RimFormationNames*> formationNames = fomNameColl->importFiles( fileList );
    if ( formationNames.size() > 1 )
    {
        RiaLogging::warning( "Multiple formation name files found in ensemble folders." );
    }

    return formationNames.front();
}
