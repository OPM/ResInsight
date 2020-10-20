#include "RicfRunOctaveScript.h"
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

#include "RicfRunOctaveScript.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimCalcScript.h"
#include "RimEclipseCase.h"
#include "RimProject.h"

#include "cafPdmFieldScriptingCapability.h"

#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RicfRunOctaveScript, "runOctaveScript" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfRunOctaveScript::RicfRunOctaveScript()
{
    CAF_PDM_InitScriptableField( &m_path, "path", QString(), "Path", "", "", "" );
    CAF_PDM_InitScriptableField( &m_caseIds, "caseIds", std::vector<int>(), "Case IDs", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfRunOctaveScript::execute()
{
    QString octavePath = RiaApplication::instance()->octavePath();

    QStringList processArguments = RimCalcScript::createCommandLineArguments( m_path() );

    std::vector<int> caseIds = m_caseIds();
    if ( caseIds.empty() )
    {
        RimProject* project = RimProject::current();
        if ( project )
        {
            auto eclipeCases = project->eclipseCases();
            for ( auto c : eclipeCases )
            {
                caseIds.push_back( c->caseId() );
            }
        }
    }

    bool ok;
    if ( caseIds.empty() )
    {
        ok = RiaApplication::instance()->launchProcess( octavePath,
                                                        processArguments,
                                                        RiaApplication::instance()->octaveProcessEnvironment() );
    }
    else
    {
        ok = RiaApplication::instance()->launchProcessForMultipleCases( octavePath,
                                                                        processArguments,
                                                                        caseIds,
                                                                        RiaApplication::instance()->octaveProcessEnvironment() );
    }

    caf::PdmScriptResponse response;
    if ( !ok )
    {
        QString error = QString( "runOctaveScript: Could not execute script %1" ).arg( m_path() );
        RiaLogging::error( error );
        response.updateStatus( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }
    else
    {
        RiaApplication::instance()->waitForProcess();
    }
    return response;
}
