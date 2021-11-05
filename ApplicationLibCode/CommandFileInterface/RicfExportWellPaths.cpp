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

#include "ExportCommands/RicExportSelectedWellPathsFeature.h"

#include "RimDialogData.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimFractureTemplate.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaWellNameComparer.h"

#include "cafCmdFeatureManager.h"
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
    using TOOLS = RicfApplicationTools;

    std::vector<RimWellPath*> wellPaths;

    // Find well paths
    {
        QStringList wellsNotFound;
        wellPaths = TOOLS::wellPathsFromNames( TOOLS::toQStringList( m_wellPathNames ), &wellsNotFound );
        if ( !wellsNotFound.empty() )
        {
            QString error( QString( "exportWellPaths: These well paths were not found: " ) +
                           wellsNotFound.join( ", " ) );
            RiaLogging::error( error );
            return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
        }
    }

    if ( wellPaths.empty() )
    {
        QString error( "No well paths found" );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }

    QString exportFolder =
        RicfCommandFileExecutor::instance()->getExportPath( RicfCommandFileExecutor::ExportType::WELLPATHS );
    if ( exportFolder.isNull() )
    {
        exportFolder = RiaApplication::instance()->createAbsolutePathFromProjectRelativePath( "wellpaths" );
    }

    caf::CmdFeatureManager* commandManager = caf::CmdFeatureManager::instance();
    auto                    feature        = dynamic_cast<RicExportSelectedWellPathsFeature*>(
        commandManager->getCommandFeature( "RicExportSelectedWellPathsFeature" ) );

    for ( const auto wellPath : wellPaths )
    {
        if ( wellPath )
        {
            feature->exportWellPath( wellPath, m_mdStepSize, exportFolder, false );
        }
    }
    return caf::PdmScriptResponse();
}
