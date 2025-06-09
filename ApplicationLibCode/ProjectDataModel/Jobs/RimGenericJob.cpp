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

#include "RimProcess.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafProgressInfo.h"

#include <QMessageBox>

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimGenericJob, "GenericJob" ); // Do not use. Abstract class

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGenericJob::RimGenericJob()
{
    CAF_PDM_InitObject( "Generic Job" );
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
void RimGenericJob::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicRunJobFeature";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGenericJob::workingDirectory() const
{
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGenericJob::execute()
{
    caf::ProgressInfo runProgress( 2, title() );

    {
        auto taskPrep = runProgress.task( "Preparing input..." );

        if ( !onPrepare() ) return false;
    }

    auto taskRun = runProgress.task( "Running job, please wait..." );

    QStringList cmdLine = command();
    if ( cmdLine.isEmpty() ) return false;

    QString cmd = cmdLine.takeFirst();

    RimProcess process;
    process.setCommand( cmd );
    if ( !cmdLine.isEmpty() ) process.addParameters( cmdLine );
    process.setWorkingDirectory( workingDirectory() );

    bool runOk = process.execute();

    onCompleted( runOk );

    if ( !runOk )
    {
        QMessageBox::critical( nullptr, title(), "Failed to run job. Check log window for additional information." );
    }
    return runOk;
}