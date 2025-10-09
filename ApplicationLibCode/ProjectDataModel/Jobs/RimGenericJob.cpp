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

#include "RiaColorTools.h"

#include "RimProcess.h"

#include "RiuGuiTheme.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiTreeAttributes.h"
#include "cafProgressInfo.h"

#include <QMessageBox>

CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimGenericJob, "GenericJob" ); // Do not use. Abstract class

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGenericJob::RimGenericJob()
    : m_percentageDone( 0.0 )
    , m_lastRunFailed( false )
    , m_isRunning( false )
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
double RimGenericJob::percentageDone() const
{
    return m_percentageDone;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGenericJob::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicRunJobFeature";
    menuBuilder << "RicDuplicateJobFeature";
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
    m_percentageDone = 0.0;

    // job preparations
    {
        caf::ProgressInfo prepProgress( 1, title(), false );

        auto prepRun = prepProgress.task( "Preparing for run, please wait..." );

        if ( !onPrepare() )
        {
            m_lastRunFailed = true;
            onProgress( m_percentageDone );
            return false;
        }
    }

    // check if we should run
    if ( !onRun() ) return false;

    QStringList cmdLine = command();
    if ( cmdLine.isEmpty() ) return false;

    m_isRunning     = true;
    m_lastRunFailed = false;
    onProgress( m_percentageDone );

    // run job
    bool runOk = false;
    {
        caf::ProgressInfo runProgress( 1, title() );

        auto taskRun = runProgress.task( "Running job, please wait..." );

        QString cmd = cmdLine.takeFirst();

        RimProcess process;
        process.setCommand( cmd );
        if ( !cmdLine.isEmpty() ) process.addParameters( cmdLine );
        process.setWorkingDirectory( workingDirectory() );
        for ( const auto& [name, value] : environment() )
        {
            process.addEnvironmentVariable( name, value );
        }

        runOk = process.execute();
    }

    m_isRunning = false;

    onCompleted( runOk );

    if ( !runOk )
    {
        m_lastRunFailed = true;
        QMessageBox::critical( nullptr, title(), "Failed to run job. Check log window for additional information." );
    }

    m_percentageDone = 100.0;
    onProgress( m_percentageDone );

    return runOk;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGenericJob::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( auto* treeItemAttribute = dynamic_cast<caf::PdmUiTreeViewItemAttribute*>( attribute ) )
    {
        if ( m_lastRunFailed )
        {
            auto tag = caf::PdmUiTreeViewItemAttribute::createTag( QColor( Qt::red ),
                                                                   RiuGuiTheme::getColorByVariableName( "backgroundColor1" ),
                                                                   "!!!" );
            treeItemAttribute->tags.push_back( std::move( tag ) );
        }
        else
        {
            if ( m_percentageDone == 0.0 ) return;

            auto tag = caf::PdmUiTreeViewItemAttribute::createTag();

            if ( m_isRunning )
            {
                tag->text = QString( "%1 %" ).arg( m_percentageDone );
            }
            else
            {
                tag->text = "Done";
            }

            double factor = m_percentageDone / 100.0;

            cvf::Color3f viewColor = cvf::Color3f( cvf::Color3f::GREEN );
            viewColor.set( viewColor.r() * factor, viewColor.g() * factor, viewColor.b() * factor );
            cvf::Color3f viewTextColor = RiaColorTools::contrastColor( viewColor );
            tag->bgColor               = QColor( RiaColorTools::toQColor( viewColor ) );
            tag->fgColor               = QColor( RiaColorTools::toQColor( viewTextColor ) );
            treeItemAttribute->tags.push_back( std::move( tag ) );
        }
    }
}
