/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RimJobCollection.h"

#include "RiaLogging.h"

#include "Rim3dView.h"
#include "RimGenericJob.h"
#include "RimGridView.h"
#include "RimProject.h"

#include "cafCmdFeatureMenuBuilder.h"

CAF_PDM_SOURCE_INIT( RimJobCollection, "JobCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimJobCollection::RimJobCollection()
{
    CAF_PDM_InitObject( "Jobs" + RiaDefines::betaFeaturePostfix(), ":/gear_icon_16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_jobs, "Jobs", "Jobs" );

    setCustomContextMenuEnabled( true );
    setDeletable( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimJobCollection::~RimJobCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGenericJob*> RimJobCollection::jobs() const
{
    return m_jobs.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimJobCollection::isEmpty()
{
    return !m_jobs.hasChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimJobCollection::addNewJob( RimGenericJob* newJob )
{
    m_jobs.push_back( newJob );
    updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimJobCollection::numberOfRunningJobs() const
{
    int nRunning = 0;
    for ( auto job : jobs() )
    {
        if ( job->isRunning() ) nRunning++;
    }

    return nRunning;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimJobCollection::deleteAllJobs()
{
    for ( auto job : jobs() )
    {
        if ( job->isRunning() )
        {
            job->stop();
            RiaLogging::info( "Stopped running job '" + job->name() );
        }
    }
    m_jobs.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimJobCollection::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicNewOpmFlowJobFeature";
}
