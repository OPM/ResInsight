/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RimGenericJob.h"

#include "cafCmdFeatureMenuBuilder.h"

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimGenericJob, "GenericJob" ); // Do not use. Abstract class

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGenericJob::RimGenericJob()
    : m_jobState( JobState::Idle )
{
    CAF_PDM_InitObject( "Generic Job" );

    CAF_PDM_InitField( &m_isChildJob, "IsChildJob", false, "Is Child Job" );
    m_isChildJob.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGenericJob::~RimGenericJob()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGenericJob::isRunning() const
{
    return ( m_jobState == JobState::Queued ) || ( m_jobState == JobState::Running );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGenericJob::JobState RimGenericJob::state() const
{
    return m_jobState;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGenericJob::setState( RimGenericJob::JobState state )
{
    m_jobState = state;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGenericJob::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    if ( isRunning() )
    {
        menuBuilder << "RicStopJobFeature";
    }
    else
    {
        menuBuilder << "RicRunJobFeature";
    }
    menuBuilder << "RicDuplicateJobFeature";
    menuBuilder << "RicViewJobLogFeature";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGenericJob::setStarted()
{
    m_startTime = QDateTime::currentDateTime();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimGenericJob::runTimeSeconds()
{
    if ( m_startTime.isValid() )
    {
        if ( isRunning() )
        {
            return m_startTime.secsTo( QDateTime::currentDateTime() );
        }
        else if ( m_stopTime.isValid() )
        {
            return m_startTime.secsTo( m_stopTime );
        }
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGenericJob::setFinished( bool runOk )
{
    m_stopTime = QDateTime::currentDateTime();
    if ( runOk )
    {
        setState( RimGenericJob::Completed );
    }
    else
    {
        setState( RimGenericJob::Failed );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGenericJob::runTimeString()
{
    double seconds = runTimeSeconds();
    int    h       = static_cast<int>( seconds / 3600 );
    int    m       = static_cast<int>( ( seconds - h * 3600 ) / 60 );
    int    s       = static_cast<int>( seconds - h * 3600 - m * 60 );

    return QString( "%1:%2:%3" ).arg( h, 2, 10, QChar( '0' ) ).arg( m, 2, 10, QChar( '0' ) ).arg( s, 2, 10, QChar( '0' ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGenericJob::isChildJob() const
{
    return m_isChildJob();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGenericJob::setIsChildJob( bool isChildJob )
{
    m_isChildJob = isChildJob;
    if ( isChildJob ) setDeletable( false );
}
