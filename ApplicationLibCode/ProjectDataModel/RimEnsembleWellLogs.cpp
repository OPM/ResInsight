/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RimEnsembleWellLogs.h"

#include "RiaLogging.h"

#include "RimWellLogFile.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimEnsembleWellLogs, "EnsembleWellLogs" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleWellLogs::RimEnsembleWellLogs()
{
    CAF_PDM_InitScriptableObject( "Ensemble Well Logs", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_wellLogFiles, "WellLogFiles", "", "", "", "" );
    m_wellLogFiles.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogs::removeWellLogFile( RimWellLogFile* summaryCase )
{
    m_wellLogFiles.removeChildObject( summaryCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogs::addWellLogFile( RimWellLogFile* summaryCase )
{
    m_wellLogFiles.push_back( summaryCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogFile*> RimEnsembleWellLogs::wellLogFiles() const
{
    return m_wellLogFiles().childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogs::loadDataAndUpdate()
{
    for ( auto& w : m_wellLogFiles )
    {
        QString errorMessage;
        if ( !w->readFile( &errorMessage ) )
        {
            RiaLogging::warning( errorMessage );
        }
    }
}
