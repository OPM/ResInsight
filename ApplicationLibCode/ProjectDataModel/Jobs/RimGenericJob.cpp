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
#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "RimJobMonitor.h"
#include "RimProcess.h"
#include "RimProcessQueue.h"

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
    , m_jobState( JobState::Idle )
    , m_errorsDetected( 0 )
    , m_warningsDetected( 0 )
    , m_process( nullptr )
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
QString RimGenericJob::workingDirectory() const
{
    return "";
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
bool RimGenericJob::stop()
{
    if ( !m_process.isNull() )
    {
        RimProcessQueue::stopProcess( m_process->ID() );
        RiaLogging::info( "Job \"" + name() + "\" stopped by user." );
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGenericJob::execute()
{
    if ( isRunning() ) return false;

    m_errorsDetected   = 0;
    m_warningsDetected = 0;
    m_percentageDone   = 0.0;
    m_process          = nullptr;
    m_jobState         = JobState::Idle;

    onProgress( m_percentageDone );

    // job preparations
    {
        caf::ProgressInfo prepProgress( 1, name(), false );

        auto prepRun = prepProgress.task( "Preparing for run, please wait..." );

        if ( !onPrepare() )
        {
            m_jobState = JobState::Failed;
            onProgress( m_percentageDone );
            return false;
        }
    }

    // check if we should run
    if ( !onRun() ) return false;

    QStringList cmdLine = command();
    if ( cmdLine.isEmpty() )
    {
        m_jobState = JobState::Failed;
        onProgress( m_percentageDone );
        return false;
    }

    // cannot delete job while running
    setDeletable( false );
    m_jobState = JobState::Queued;

    m_process = new RimProcess( true, new RimJobMonitor( this ) );

    // build process to run
    QString cmd = cmdLine.takeFirst();
    m_process->setCommand( cmd );
    if ( !cmdLine.isEmpty() ) m_process->addParameters( cmdLine );
    m_process->setWorkingDirectory( workingDirectory() );
    for ( const auto& [name, value] : environment() )
    {
        m_process->addEnvironmentVariable( name, value );
    }

    RimProcessQueue::queueProcess( m_process );
    onProgress( m_percentageDone );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGenericJob::setFinished( bool runOk )
{
    m_jobState = runOk ? JobState::Completed : JobState::Failed;

    m_percentageDone = 100.0;
    onProgress( m_percentageDone );
    setDeletable( true );

    onCompleted( runOk );

    return runOk;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGenericJob::setStarted()
{
    m_jobState = RimGenericJob::JobState::Running;
    onProgress( m_percentageDone );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGenericJob::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    static auto warnColor = QColor( RiaColorTools::toQColor( cvf::Color3f( cvf::Color3f::DARK_YELLOW ) ) );
    static auto contrastWarnColor =
        QColor( RiaColorTools::toQColor( RiaColorTools::contrastColor( cvf::Color3f( cvf::Color3f::DARK_YELLOW ) ) ) );

    static auto waitColor = QColor( RiaColorTools::toQColor( cvf::Color3f( cvf::Color3f::LIGHT_GRAY ) ) );
    static auto contrastWaitColor =
        QColor( RiaColorTools::toQColor( RiaColorTools::contrastColor( cvf::Color3f( cvf::Color3f::LIGHT_GRAY ) ) ) );

    if ( auto* treeItemAttribute = dynamic_cast<caf::PdmUiTreeViewItemAttribute*>( attribute ) )
    {
        if ( m_jobState == JobState::Failed )
        {
            auto txt = m_errorsDetected > 0 ? QString( "[%1]" ).arg( m_errorsDetected ) : "!!!";
            auto tag =
                caf::PdmUiTreeViewItemAttribute::createTag( QColor( Qt::red ), RiuGuiTheme::getColorByVariableName( "backgroundColor1" ), txt );
            treeItemAttribute->tags.push_back( std::move( tag ) );
        }
        else if ( m_jobState == JobState::Queued )
        {
            auto tag     = caf::PdmUiTreeViewItemAttribute::createTag();
            tag->text    = "Waiting...";
            tag->bgColor = waitColor;
            tag->fgColor = contrastWaitColor;
            treeItemAttribute->tags.push_back( std::move( tag ) );
        }
        else if ( ( m_jobState == JobState::Running ) || ( m_jobState == JobState::Completed ) )
        {
            auto tag = caf::PdmUiTreeViewItemAttribute::createTag();

            if ( m_jobState == JobState::Running )
            {
                tag->text = QString( "%1 %" ).arg( m_percentageDone, 0, 'f', 1 );
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
        if ( m_warningsDetected > 0 )
        {
            auto warnTxt = QString( "[%1]" ).arg( m_warningsDetected );

            auto warnTag = caf::PdmUiTreeViewItemAttribute::createTag( warnColor, contrastWarnColor, warnTxt );
            treeItemAttribute->tags.push_back( std::move( warnTag ) );
            auto iconTag  = caf::PdmUiTreeViewItemAttribute::createTag();
            iconTag->icon = caf::IconProvider( ":/warning.png" );
            treeItemAttribute->tags.push_back( std::move( iconTag ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QStringList RimGenericJob::jobLog() const
{
    if ( m_process.isNull() ) return QStringList();
    return m_process->stdOut();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGenericJob::matchesKeyValue( const QString& key, const QString& value ) const
{
    return false;
}