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
#include "RiaLogging.h"

#include "RimJobMonitor.h"
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
    , m_process( nullptr )
    , m_errorsDetected( 0 )
    , m_warningsDetected( 0 )
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
    return m_isRunning;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGenericJob::stop()
{
    if ( !m_process.isNull() )
    {
        m_process->terminate();
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
    m_process          = nullptr;
    m_percentageDone   = 0.0;

    // job preparations
    {
        caf::ProgressInfo prepProgress( 1, name(), false );

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

    // cannot delete job while running
    setDeletable( false );

    m_process = new RimProcess( true, new RimJobMonitor( this ) );

    m_isRunning     = true;
    m_lastRunFailed = false;

    onProgress( m_percentageDone );

    // build process to run
    QString cmd = cmdLine.takeFirst();
    m_process->setCommand( cmd );
    if ( !cmdLine.isEmpty() ) m_process->addParameters( cmdLine );
    m_process->setWorkingDirectory( workingDirectory() );
    for ( const auto& [name, value] : environment() )
    {
        m_process->addEnvironmentVariable( name, value );
    }

    // run process
    bool startOk = m_process->start();
    if ( !startOk )
    {
        onCompleted( false );
        m_lastRunFailed = true;
        m_isRunning     = false;
        setDeletable( true );
        QMessageBox::critical( nullptr, name(), "Failed to start job. Check log window for additional information." );
    }

    return startOk;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGenericJob::setFinished( bool runOk )
{
    m_isRunning = false;

    m_lastRunFailed = !runOk;

    m_percentageDone = 100.0;
    onProgress( m_percentageDone );
    setDeletable( true );

    onCompleted( runOk );

    return runOk;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGenericJob::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    static auto warnColor = QColor( RiaColorTools::toQColor( cvf::Color3f( cvf::Color3f::DARK_YELLOW ) ) );
    static auto contrastWarnColor =
        QColor( RiaColorTools::toQColor( RiaColorTools::contrastColor( cvf::Color3f( cvf::Color3f::DARK_YELLOW ) ) ) );

    if ( auto* treeItemAttribute = dynamic_cast<caf::PdmUiTreeViewItemAttribute*>( attribute ) )
    {
        if ( m_lastRunFailed )
        {
            auto txt = m_errorsDetected > 0 ? QString( "[%1]" ).arg( m_errorsDetected ) : "!!!";
            auto tag =
                caf::PdmUiTreeViewItemAttribute::createTag( QColor( Qt::red ), RiuGuiTheme::getColorByVariableName( "backgroundColor1" ), txt );
            treeItemAttribute->tags.push_back( std::move( tag ) );
        }
        else
        {
            if ( ( m_percentageDone == 0.0 ) && ( !m_isRunning ) ) return;

            auto tag = caf::PdmUiTreeViewItemAttribute::createTag();

            if ( m_isRunning )
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
