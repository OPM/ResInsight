/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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
#include "RicfImportWellPaths.h"

#include "WellPathCommands/RicImportWellPaths.h"

#include "RimFileWellPath.h"

#include <QDir>
#include <QFileInfo>
#include <QStringList>

CAF_PDM_SOURCE_INIT( RicfImportWellPathsResult, "importWellPathsResult" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfImportWellPathsResult::RicfImportWellPathsResult()
{
    CAF_PDM_InitObject( "well_path_result", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &wellPathNames, "wellPathNames", "", "", "", "" );
}

CAF_PDM_SOURCE_INIT( RicfImportWellPaths, "importWellPaths" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfImportWellPaths::RicfImportWellPaths()
{
    RICF_InitFieldNoDefault( &m_wellPathFolder, "wellPathFolder", "", "", "", "" );
    RICF_InitFieldNoDefault( &m_wellPathFiles, "wellPathFiles", "", "", "", "" );
}

RicfCommandResponse RicfImportWellPaths::execute()
{
    QStringList errorMessages, warningMessages;
    QStringList wellPathFiles;

    QDir wellPathFolder( m_wellPathFolder );

    if ( wellPathFolder.exists() )
    {
        QStringList nameFilters;
        nameFilters << RicImportWellPaths::wellPathNameFilters();
        QStringList relativePaths = wellPathFolder.entryList( nameFilters, QDir::Files | QDir::NoDotAndDotDot );
        for ( QString relativePath : relativePaths )
        {
            wellPathFiles.push_back( wellPathFolder.absoluteFilePath( relativePath ) );
        }
    }
    else
    {
        errorMessages << ( m_wellPathFolder() + " does not exist" );
    }

    for ( QString wellPathFile : m_wellPathFiles() )
    {
        if ( QFileInfo::exists( wellPathFile ) )
        {
            wellPathFiles.push_back( wellPathFile );
        }
        else
        {
            errorMessages << ( wellPathFile + " does not exist" );
        }
    }

    RicfCommandResponse response;
    if ( !wellPathFiles.empty() )
    {
        std::vector<RimFileWellPath*> importedWellPaths = RicImportWellPaths::importWellPaths( wellPathFiles,
                                                                                               &warningMessages );
        if ( !importedWellPaths.empty() )
        {
            RicfImportWellPathsResult* wellPathsResult = new RicfImportWellPathsResult;
            for ( RimFileWellPath* wellPath : importedWellPaths )
            {
                wellPathsResult->wellPathNames.v().push_back( wellPath->name() );
            }

            response.setResult( wellPathsResult );
        }
    }
    else
    {
        warningMessages << "No well paths found";
    }

    for ( QString warningMessage : warningMessages )
    {
        response.updateStatus( RicfCommandResponse::COMMAND_WARNING, warningMessage );
    }

    for ( QString errorMessage : errorMessages )
    {
        response.updateStatus( RicfCommandResponse::COMMAND_ERROR, errorMessage );
    }

    return response;
}
