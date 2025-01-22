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
QString RimFormationTools::formationFolderFromCaseFileName( const QString caseFileName )
{
    QFileInfo fi( caseFileName );

    auto formationFolder = QDir( fi.dir().path() + "/../../" );
    return formationFolder.absolutePath();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFormationNames* RimFormationTools::loadFormationNamesFromFolder( const QString folderName )
{
    QStringList filters;
    filters << "*.lyr";

    QStringList fileList = caf::Utils::getFilesInDirectory( folderName, filters, true /*absolute filename*/ );
    if ( fileList.isEmpty() ) return nullptr;

    RimFormationNamesCollection* fomNameColl = new RimFormationNamesCollection();

    // For each file, find existing Formation names item, or create new
    std::vector<RimFormationNames*> formationNames = fomNameColl->importFiles( fileList );

    if ( formationNames.size() > 1 )
    {
        RiaLogging::warning( QString( "Multiple formation name files found in ensemble folder %1" ).arg( folderName ) );
    }

    return formationNames.back();
}
