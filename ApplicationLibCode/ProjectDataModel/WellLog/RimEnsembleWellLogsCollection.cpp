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

#include "RimEnsembleWellLogsCollection.h"

#include "RimEnsembleWellLogs.h"

#include "cafProgressInfo.h"

CAF_PDM_SOURCE_INIT( RimEnsembleWellLogsCollection, "EnsembleWellLogsCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleWellLogsCollection::RimEnsembleWellLogsCollection()
{
    CAF_PDM_InitObject( "Ensemble Well Logs", ":/LasFile16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_ensembleWellLogs, "EnsembleWellLogsCollection", "" );
    m_ensembleWellLogs.uiCapability()->setUiTreeHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleWellLogsCollection::~RimEnsembleWellLogsCollection()
{
    m_ensembleWellLogs.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogsCollection::addEnsembleWellLogs( RimEnsembleWellLogs* ensembleWellLogs )
{
    m_ensembleWellLogs.push_back( ensembleWellLogs );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEnsembleWellLogs*> RimEnsembleWellLogsCollection::ensembleWellLogs() const
{
    std::vector<RimEnsembleWellLogs*> ensembleWellLogs;
    for ( const auto& e : m_ensembleWellLogs )
    {
        ensembleWellLogs.push_back( e );
    }
    return ensembleWellLogs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleWellLogsCollection::loadDataAndUpdate()
{
    for ( const auto& e : m_ensembleWellLogs )
    {
        e->loadDataAndUpdate();
    }
}
