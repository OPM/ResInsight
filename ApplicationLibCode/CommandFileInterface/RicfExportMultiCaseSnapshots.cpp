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

#include "RicfExportMultiCaseSnapshots.h"

#include "RicfCommandFileExecutor.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaProjectModifier.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RicfExportMultiCaseSnapshots, "exportMultiCaseSnapshots" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfExportMultiCaseSnapshots::RicfExportMultiCaseSnapshots()
{
    CAF_PDM_InitScriptableField( &m_gridListFile, "gridListFile", QString(), "Grid List File" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfExportMultiCaseSnapshots::execute()
{
    RiaGuiApplication* app = RiaGuiApplication::instance();
    if ( !app )
    {
        QString error( "exportMultiCaseSnapshots: Requires GUI Application" );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }
    if ( m_gridListFile().isNull() )
    {
        QString error( "exportMultiCaseSnapshots: Required parameter gridListFile." );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }

    QString lastProjectPath = RicfCommandFileExecutor::instance()->getLastProjectPath();
    if ( lastProjectPath.isNull() )
    {
        QString error( "exportMultiCaseSnapshots: 'openProject' must be called before 'exportMultiCaseSnapshots' to "
                       "specify project file to replace cases in." );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }

    std::vector<QString> listFileNames = RiaApplication::readFileListFromTextFile( m_gridListFile() );
    app->runMultiCaseSnapshots( lastProjectPath,
                                listFileNames,
                                RicfCommandFileExecutor::instance()->getExportPath(
                                    RicfCommandFileExecutor::ExportType::SNAPSHOTS ) );

    return caf::PdmScriptResponse();
}
