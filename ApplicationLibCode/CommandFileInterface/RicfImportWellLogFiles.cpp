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

#include "RicfCommandForwarding.h"

#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPathCollection.h"
#include "RimcDataContainerString.h"
#include "RimcWellPathCollection.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RicfImportWellLogFilesResult, "importWellLogFilesResult" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfImportWellLogFilesResult::RicfImportWellLogFilesResult()
{
    CAF_PDM_InitObject( "well_log_files_result" );
    CAF_PDM_InitFieldNoDefault( &wellPathNames, "wellPathNames", "" );
}

CAF_PDM_SOURCE_INIT( RicfImportWellLogFiles, "importWellLogFiles" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfImportWellLogFiles::RicfImportWellLogFiles()
{
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellLogFileFolder, "wellLogFolder", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellLogFilePaths, "wellLogFiles", "" );
}

caf::PdmScriptResponse RicfImportWellLogFiles::execute()
{
    const QString commandName = classKeyword();

    RimProject* project = RimProject::current();
    if ( !project || !project->activeOilField() || !project->activeOilField()->wellPathCollection() )
    {
        return RicfForwarding::errorResponse( "No well path collection available", commandName );
    }

    RimWellPathCollection_importWellLogFiles method( project->activeOilField()->wellPathCollection() );
    method.setWellLogFiles( m_wellLogFilePaths() );
    method.setWellLogFolder( m_wellLogFileFolder() );

    auto result = method.execute();

    caf::PdmScriptResponse response;
    for ( const QString& warningMessage : method.warnings() )
    {
        response.updateStatus( caf::PdmScriptResponse::COMMAND_WARNING, warningMessage );
    }

    if ( !result )
    {
        // Legacy behavior: "no files found" is a warning, missing files are errors
        if ( result.error() == "No well log files found" )
        {
            response.updateStatus( caf::PdmScriptResponse::COMMAND_WARNING, result.error() );
        }
        else
        {
            response.updateStatus( caf::PdmScriptResponse::COMMAND_ERROR, result.error() );
        }
        return response;
    }

    auto* names = dynamic_cast<RimcDataContainerString*>( result.value() );
    if ( names && !names->m_stringValues().empty() )
    {
        auto* filesResult          = new RicfImportWellLogFilesResult;
        filesResult->wellPathNames = names->m_stringValues();
        response.setResult( filesResult );
    }
    delete names;

    return response;
}
