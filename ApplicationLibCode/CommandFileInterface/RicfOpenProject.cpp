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

#include "RicfOpenProject.h"

#include "RicfCommandFileExecutor.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "cafPdmFieldScriptingCapability.h"

#include <QDir>
#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RicfOpenProject, "openProject" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfOpenProject::RicfOpenProject()
{
    CAF_PDM_InitScriptableField( &m_path, "path", QString(), "Path" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfOpenProject::execute()
{
    QString   projectPath = m_path;
    QFileInfo projectPathInfo( projectPath );
    if ( !projectPathInfo.exists() )
    {
        QDir startDir( RiaApplication::instance()->startDir() );
        projectPath = startDir.absoluteFilePath( m_path );
    }
    bool ok = RiaApplication::instance()->loadProject( projectPath );
    if ( !ok )
    {
        QString errMsg = QString( "openProject: Unable to open project at %1" ).arg( m_path() );
        RiaLogging::error( errMsg );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, errMsg );
    }

    RicfCommandFileExecutor::instance()->setLastProjectPath( projectPath );

    return caf::PdmScriptResponse();
}
