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

#include "RicfReplaceSourceCases.h"

#include "RicfCommandFileExecutor.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaProjectModifier.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RicfReplaceSourceCases, "replaceSourceCases" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfReplaceSourceCases::RicfReplaceSourceCases()
{
    CAF_PDM_InitScriptableField( &m_caseGroupId, "caseGroupId", -1, "Case Group ID", "", "", "" );
    CAF_PDM_InitScriptableField( &m_gridListFile, "gridListFile", QString(), "Grid List File", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfReplaceSourceCases::execute()
{
    if ( m_gridListFile().isNull() )
    {
        QString error( "replaceSourceCases: Required parameter gridListFile." );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }

    QString lastProjectPath = RicfCommandFileExecutor::instance()->getLastProjectPath();
    if ( lastProjectPath.isNull() )
    {
        QString error( "replaceSourceCases: 'openProject' must be called before 'replaceSourceCases' to specify "
                       "project file to replace cases in." );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }

    cvf::ref<RiaProjectModifier> projectModifier = new RiaProjectModifier;

    std::vector<QString> listFileNames = RiaApplication::readFileListFromTextFile( m_gridListFile() );
    if ( m_caseGroupId() == -1 )
    {
        projectModifier->setReplaceSourceCasesFirstOccurrence( listFileNames );
    }
    else
    {
        projectModifier->setReplaceSourceCasesById( m_caseGroupId(), listFileNames );
    }

    if ( !RiaApplication::instance()->loadProject( lastProjectPath,
                                                   RiaApplication::ProjectLoadAction::PLA_CALCULATE_STATISTICS,
                                                   projectModifier.p() ) )
    {
        QString error( "Could not reload project" );
        RiaLogging::error( error );
        return caf::PdmScriptResponse( caf::PdmScriptResponse::COMMAND_ERROR, error );
    }
    return caf::PdmScriptResponse();
}
