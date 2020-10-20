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
#include "RicfImportWellLogFiles.h"

#include "WellLogCommands/RicWellLogsImportFileFeature.h"

#include "RiaApplication.h"
#include "RimWellLogFile.h"

#include "cafPdmFieldScriptingCapability.h"

#include <QDir>
#include <QFileInfo>
#include <QStringList>

CAF_PDM_SOURCE_INIT( RicfImportWellLogFilesResult, "importWellLogFilesResult" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfImportWellLogFilesResult::RicfImportWellLogFilesResult()
{
    CAF_PDM_InitObject( "well_log_files_result", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &wellPathNames, "wellPathNames", "", "", "", "" );
}

CAF_PDM_SOURCE_INIT( RicfImportWellLogFiles, "importWellLogFiles" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfImportWellLogFiles::RicfImportWellLogFiles()
{
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellLogFileFolder, "wellLogFolder", "", "", "", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellLogFilePaths, "wellLogFiles", "", "", "", "" );
}

caf::PdmScriptResponse RicfImportWellLogFiles::execute()
{
    QStringList errorMessages, warningMessages;
    QStringList wellLogFilePaths;

    QDir wellLogDir;
    if ( m_wellLogFileFolder().isEmpty() )
    {
        wellLogDir = QDir( RiaApplication::instance()->startDir() );
    }
    else
    {
        wellLogDir = QDir( m_wellLogFileFolder );
    }

    if ( !m_wellLogFileFolder().isEmpty() )
    {
        QStringList nameFilters;
        nameFilters << RicWellLogsImportFileFeature::wellLogFileNameFilters();
        QStringList relativePaths = wellLogDir.entryList( nameFilters, QDir::Files | QDir::NoDotAndDotDot );
        for ( QString relativePath : relativePaths )
        {
            wellLogFilePaths.push_back( wellLogDir.absoluteFilePath( relativePath ) );
        }
    }
    else
    {
        errorMessages << ( m_wellLogFileFolder() + " doesn't exist" );
    }

    for ( QString wellLogFilePath : m_wellLogFilePaths() )
    {
        if ( QFileInfo::exists( wellLogFilePath ) )
        {
            wellLogFilePaths.push_back( wellLogFilePath );
        }
        else if ( QFileInfo::exists( wellLogDir.absoluteFilePath( wellLogFilePath ) ) )
        {
            wellLogFilePaths.push_back( wellLogDir.absoluteFilePath( wellLogFilePath ) );
        }
        else
        {
            errorMessages << ( wellLogFilePath + " doesn't exist" );
        }
    }

    caf::PdmScriptResponse response;

    if ( !wellLogFilePaths.empty() )
    {
        std::vector<RimWellLogFile*> importedWellLogFiles =
            RicWellLogsImportFileFeature::importWellLogFiles( wellLogFilePaths, &warningMessages );
        if ( !importedWellLogFiles.empty() )
        {
            RicfImportWellLogFilesResult* result = new RicfImportWellLogFilesResult;
            for ( RimWellLogFile* wellLogFile : importedWellLogFiles )
            {
                result->wellPathNames.v().push_back( wellLogFile->wellName() );
            }
            response.setResult( result );
        }
    }
    else
    {
        warningMessages << "No well log files found";
    }

    for ( QString warningMessage : warningMessages )
    {
        response.updateStatus( caf::PdmScriptResponse::COMMAND_WARNING, warningMessage );
    }

    for ( QString errorMessage : errorMessages )
    {
        response.updateStatus( caf::PdmScriptResponse::COMMAND_ERROR, errorMessage );
    }

    return response;
}
