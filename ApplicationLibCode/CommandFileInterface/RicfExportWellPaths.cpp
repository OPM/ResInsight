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

#include "RicfExportWellPaths.h"

#include "RicfApplicationTools.h"
#include "RicfCommandFileExecutor.h"
#include "RicfCommandForwarding.h"

#include "RiaApplication.h"

#include "RimWellPath.h"
#include "RimcWellPath.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RicfExportWellPaths, "exportWellPaths" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfExportWellPaths::RicfExportWellPaths()
{
    CAF_PDM_InitScriptableField( &m_wellPathNames, "wellPathNames", std::vector<QString>(), "Well Path Names" );
    CAF_PDM_InitScriptableField( &m_mdStepSize, "mdStepSize", 5.0, "MD Step Size" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfExportWellPaths::execute()
{
    const QString commandName = classKeyword();

    // Empty name list means all well paths
    QStringList               wellsNotFound;
    std::vector<RimWellPath*> wellPaths =
        RicfApplicationTools::wellPathsFromNames( RicfApplicationTools::toQStringList( m_wellPathNames ), &wellsNotFound );
    if ( !wellsNotFound.empty() )
    {
        return RicfForwarding::errorResponse( "These well paths were not found: " + wellsNotFound.join( ", " ), commandName );
    }
    if ( wellPaths.empty() ) return RicfForwarding::errorResponse( "No well paths found", commandName );

    // Resolve the export folder from the command file executor state. The Rimc method requires an explicit folder.
    QString exportFolder = RicfCommandFileExecutor::instance()->getExportPath( RicfCommandFileExecutor::ExportType::WELLPATHS );
    if ( exportFolder.isNull() )
    {
        exportFolder = RiaApplication::instance()->createAbsolutePathFromProjectRelativePath( "wellpaths" );
    }

    for ( RimWellPath* wellPath : wellPaths )
    {
        if ( !wellPath ) continue;

        RimWellPath_exportGeometry method( wellPath );
        method.setExportFolder( exportFolder );
        method.setMdStepSize( m_mdStepSize() );

        auto result = method.execute();
        if ( !result ) return RicfForwarding::errorResponse( result.error(), commandName );
    }

    return caf::PdmScriptResponse();
}
