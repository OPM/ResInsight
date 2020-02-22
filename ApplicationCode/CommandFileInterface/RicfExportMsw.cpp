/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicfExportMsw.h"

#include "RicfApplicationTools.h"
#include "RicfCommandFileExecutor.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimFishbonesCollection.h"
#include "RimFishbonesMultipleSubs.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "CompletionExportCommands/RicExportCompletionDataSettingsUi.h"
#include "CompletionExportCommands/RicWellPathExportMswCompletionsImpl.h"

CAF_PDM_SOURCE_INIT( RicfExportMsw, "exportMsw" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfExportMsw::RicfExportMsw()
{
    RICF_InitField( &m_caseId, "caseId", -1, "Case ID", "", "", "" );
    RICF_InitField( &m_wellPathName, "wellPath", QString(), "Well Path Name", "", "", "" );
    RICF_InitField( &m_includePerforations, "includePerforations", true, "Include Perforations", "", "", "" );
    RICF_InitField( &m_includeFishbones, "includeFishbones", true, "Include Fishbones", "", "", "" );
    RICF_InitField( &m_includeFractures, "includeFractures", true, "Include Fractures", "", "", "" );
    RICF_InitField( &m_fileSplit, "fileSplit", RicExportCompletionDataSettingsUi::ExportSplitType(), "File Split", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfCommandResponse RicfExportMsw::execute()
{
    using TOOLS = RicfApplicationTools;

    RicExportCompletionDataSettingsUi exportSettings;

    auto eclipseCase = TOOLS::caseFromId( m_caseId() );
    if ( !eclipseCase )
    {
        QString error = QString( "exportMsw: Could not find case with ID %1." ).arg( m_caseId() );
        RiaLogging::error( error );
        return RicfCommandResponse( RicfCommandResponse::COMMAND_ERROR, error );
    }

    QString exportFolder = RicfCommandFileExecutor::instance()->getExportPath( RicfCommandFileExecutor::COMPLETIONS );
    if ( exportFolder.isNull() )
    {
        exportFolder = RiaApplication::instance()->createAbsolutePathFromProjectRelativePath( "completions" );
    }
    exportSettings.caseToApply         = eclipseCase;
    exportSettings.folder              = exportFolder;
    exportSettings.includePerforations = m_includePerforations;
    exportSettings.includeFishbones    = m_includeFishbones;
    exportSettings.includeFractures    = m_includeFractures;
    exportSettings.fileSplit           = m_fileSplit;

    RimWellPath* wellPath = RiaApplication::instance()->project()->wellPathByName( m_wellPathName );
    if ( !wellPath )
    {
        QString error = QString( "exportMsw: Could not find well path with name %1" ).arg( m_wellPathName() );
        RiaLogging::error( error );
        return RicfCommandResponse( RicfCommandResponse::COMMAND_ERROR, error );
    }

    RicWellPathExportMswCompletionsImpl::exportWellSegmentsForAllCompletions( exportSettings, {wellPath} );

    return RicfCommandResponse();
}
