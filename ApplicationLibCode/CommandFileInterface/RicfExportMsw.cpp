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

#include "RicfCommandFileExecutor.h"
#include "RicfCommandForwarding.h"

#include "RiaApplication.h"

#include "RimEclipseCase.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimcEclipseCase.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RicfExportMsw, "exportMsw" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfExportMsw::RicfExportMsw()
{
    CAF_PDM_InitScriptableField( &m_caseId, "caseId", -1, "Case ID" );
    CAF_PDM_InitScriptableField( &m_wellPathName, "wellPath", QString(), "Well Path Name" );
    CAF_PDM_InitScriptableField( &m_includePerforations, "includePerforations", true, "Include Perforations" );
    CAF_PDM_InitScriptableField( &m_includeFishbones, "includeFishbones", true, "Include Fishbones" );
    CAF_PDM_InitScriptableField( &m_includeFractures, "includeFractures", true, "Include Fractures" );
    CAF_PDM_InitScriptableField( &m_fileSplit, "fileSplit", RicExportCompletionDataSettingsUi::ExportSplitType(), "File Split" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfExportMsw::execute()
{
    const QString commandName = classKeyword();

    auto rimCase = RicfForwarding::findCase( m_caseId() );
    if ( !rimCase ) return RicfForwarding::errorResponse( rimCase.error(), commandName );

    auto* eclipseCase = dynamic_cast<RimEclipseCase*>( rimCase.value() );
    if ( !eclipseCase )
    {
        return RicfForwarding::errorResponse( QString( "Case with ID %1 is not an Eclipse case" ).arg( m_caseId() ), commandName );
    }

    RimWellPath* wellPath = RimProject::current()->wellPathByName( m_wellPathName() );
    if ( !wellPath )
    {
        return RicfForwarding::errorResponse( QString( "Could not find well path with name %1" ).arg( m_wellPathName() ), commandName );
    }

    // Resolve the export folder from the command file executor state. The Rimc method requires an explicit folder.
    QString exportFolder = RicfCommandFileExecutor::instance()->getExportPath( RicfCommandFileExecutor::ExportType::COMPLETIONS );
    if ( exportFolder.isNull() )
    {
        exportFolder = RiaApplication::instance()->createAbsolutePathFromProjectRelativePath( "completions" );
    }

    RimEclipseCase_exportMswCompletions method( eclipseCase );
    method.setWellPaths( { wellPath } );
    method.setExportFolder( exportFolder );
    method.setFileSplit( m_fileSplit() );
    method.setIncludePerforations( m_includePerforations() );
    method.setIncludeFishbones( m_includeFishbones() );
    method.setIncludeFractures( m_includeFractures() );

    return RicfForwarding::toScriptResponse( method.execute(), commandName );
}
