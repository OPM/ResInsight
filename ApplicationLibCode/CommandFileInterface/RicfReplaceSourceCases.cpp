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
#include "RicfCommandForwarding.h"

#include "RiaApplication.h"

#include "RimIdenticalGridCaseGroup.h"
#include "RimcIdenticalGridCaseGroup.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RicfReplaceSourceCases, "replaceSourceCases" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicfReplaceSourceCases::RicfReplaceSourceCases()
{
    CAF_PDM_InitScriptableField( &m_caseGroupId, "caseGroupId", -1, "Case Group ID" );
    CAF_PDM_InitScriptableField( &m_gridListFile, "gridListFile", QString(), "Grid List File" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmScriptResponse RicfReplaceSourceCases::execute()
{
    const QString commandName = classKeyword();

    if ( m_gridListFile().isNull() ) return RicfForwarding::errorResponse( "Required parameter gridListFile.", commandName );

    // The legacy command reloads the project opened by 'openProject', which is not necessarily saved.
    QString lastProjectPath = RicfCommandFileExecutor::instance()->getLastProjectPath();
    if ( lastProjectPath.isNull() )
    {
        return RicfForwarding::errorResponse( "'openProject' must be called before 'replaceSourceCases' to specify project file to replace "
                                              "cases in.",
                                              commandName );
    }

    auto caseGroup = RicfForwarding::findCaseGroupOrFirst( m_caseGroupId() );
    if ( !caseGroup ) return RicfForwarding::errorResponse( caseGroup.error(), commandName );

    RimIdenticalGridCaseGroup_replaceSourceCases method( caseGroup.value() );
    method.setGridFiles( RiaApplication::readFileListFromTextFile( m_gridListFile() ) );
    method.setProjectFile( lastProjectPath );

    return RicfForwarding::toScriptResponse( method.execute(), commandName );
}
