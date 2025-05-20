/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicHelpFeatures.h"

#include "SummaryPlotCommands/RicSummaryPlotFeatureImpl.h"

#include "RiaApplication.h"
#include "RiaBaseDefs.h"
#include "RiaNetworkTools.h"
#include "RiaPreferences.h"
#include "RiaVersionInfo.h"

#include "RiuMainWindow.h"

#include "cafAboutDialog.h"
#include "cafSelectionManager.h"
#include "cafViewer.h"

#include <QAction>
#include <QDesktopServices>
#include <QErrorMessage>
#include <QSslSocket>
#include <QUrl>

CAF_CMD_SOURCE_INIT( RicHelpAboutFeature, "RicHelpAboutFeature" );
CAF_CMD_SOURCE_INIT( RicHelpCommandLineFeature, "RicHelpCommandLineFeature" );
CAF_CMD_SOURCE_INIT( RicHelpSummaryCommandLineFeature, "RicHelpSummaryCommandLineFeature" );
CAF_CMD_SOURCE_INIT( RicHelpOpenUsersGuideFeature, "RicHelpOpenUsersGuideFeature" );
CAF_CMD_SOURCE_INIT( RicSearchHelpFeature, "RicSearchHelpFeature" );
CAF_CMD_SOURCE_INIT( RicSearchIssuesHelpFeature, "RicSearchIssuesHelpFeature" );
CAF_CMD_SOURCE_INIT( RicCreateNewIssueHelpFeature, "RicCreateNewIssueHelpFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHelpAboutFeature::onActionTriggered( bool isChecked )
{
    disableModelChangeContribution();

    caf::AboutDialog dlg( nullptr );

    dlg.setApplicationName( RI_APPLICATION_NAME );
    dlg.setApplicationVersion( RiaApplication::getVersionStringApp( true ) );
    dlg.setCopyright( "Copyright Equinor ASA, Ceetron Solutions AS, Ceetron AS" );
    dlg.showQtVersion( false );
#ifdef _DEBUG
    dlg.setIsDebugBuild( true );
#endif

    dlg.addVersionEntry( " ", "ResInsight is made available under the GNU General Public License v. 3" );
    dlg.addVersionEntry( " ", "See http://www.gnu.org/licenses/gpl.html" );
    dlg.addVersionEntry( " ", " " );

    QStringList activeFeatures;
#ifdef USE_ODB_API
    activeFeatures += "  Geomech";
#endif
#ifdef USE_HDF5
    activeFeatures += "  HDF5";
#endif

    if ( !activeFeatures.isEmpty() )
    {
        dlg.addVersionEntry( " ", "Features" );

        for ( const auto& feature : activeFeatures )
        {
            dlg.addVersionEntry( " ", feature );
        }
    }

    dlg.addVersionEntry( " ", " " );
    dlg.addVersionEntry( " ", "Technical Information" );
    dlg.addVersionEntry( " ", QString( "   Qt " ) + qVersion() );
    dlg.addVersionEntry( " ", QString( "   " ) + caf::AboutDialog::versionStringForcurrentOpenGLContext() );
    dlg.addVersionEntry( " ", caf::Viewer::isShadersSupported() ? "   Hardware OpenGL" : "   Software OpenGL" );

    if ( !QString( RESINSIGHT_OCTAVE_VERSION ).isEmpty() )
        dlg.addVersionEntry( " ", QString( "   Octave " ) + QString( RESINSIGHT_OCTAVE_VERSION ) );

    bool isAbleToUseSsl = false;
    bool isSslSupported = false;
#ifndef QT_NO_OPENSSL
    isAbleToUseSsl = true;
    isSslSupported = QSslSocket::supportsSsl();
#endif

    {
        QString txt;

        if ( isAbleToUseSsl )
        {
            txt = "   Use of SSL is available";
            if ( isSslSupported )
            {
                txt += " and supported : ";
                txt += QSslSocket::sslLibraryVersionString();
                txt += ", Build: " + QSslSocket::sslLibraryBuildVersionString();
            }
            else
            {
                txt += ", but not supported";
            }
        }
        else
        {
            txt = "   SSL is not available";
        }

        dlg.addVersionEntry( " ", txt );
    }

    if ( RiaApplication::enableDevelopmentFeatures() )
    {
        QString vendor( "Unknown" );
        QString render( "Unknown" );

        {
            char* str = (char*)glGetString( GL_VENDOR );

            if ( str )
            {
                vendor = str;
            }
        }

        {
            char* str = (char*)glGetString( GL_RENDERER );

            if ( str )
            {
                render = str;
            }
        }

        dlg.addVersionEntry( " ", QString( "   " ) + vendor + " : " + render );
    }

    QString buildSystemId = RESINSIGHT_BUILD_SYSTEM_ID;
    if ( !buildSystemId.isEmpty() )
    {
        dlg.addVersionEntry( " ", QString( "    Build Server : " ) + buildSystemId );
    }

    QString compiledUsingPythonVersion = RESINSIGHT_PYTHON_VERSION;
    if ( !compiledUsingPythonVersion.isEmpty() )
    {
        auto pythonRuntimePath    = RiaPreferences::current()->pythonExecutable();
        auto pythonRuntimeVersion = RicHelpAboutFeature::getPythonVersion( pythonRuntimePath );

        dlg.addVersionEntry( " ", "" );
        dlg.addVersionEntry( " ", "Python" );
        dlg.addVersionEntry( " ", QString( "  Compiled with: Python " ) + compiledUsingPythonVersion );
        dlg.addVersionEntry( " ", QString( "  Runtime binary: " ) + pythonRuntimePath );
        dlg.addVersionEntry( " ", QString( "  Runtime version: " ) + pythonRuntimeVersion );
    }

    dlg.addVersionEntry( " ", "" );
    QString nowDate = QDateTime::currentDateTime().toString( "yyyy-MMM-dd" );
    dlg.addVersionEntry( "", QString( "Build date: " ) + nowDate );

    dlg.create();
    dlg.resize( 300, 200 );

    dlg.exec();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHelpAboutFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "&About" );
    actionToSetup->setIcon( QIcon( ":/HelpCircle.svg" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicHelpAboutFeature::getPythonVersion( const QString& pathToPythonExecutable )
{
    QStringList arguments;
    arguments << "--version";

    QProcess process;
    process.setProgram( pathToPythonExecutable );
    process.setArguments( arguments );

    process.start();
    process.waitForFinished();

    QByteArray output = process.readAllStandardOutput();

    QString versionString = QString( output ).trimmed();
    return versionString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHelpCommandLineFeature::onActionTriggered( bool isChecked )
{
    disableModelChangeContribution();

    RiaApplication* app  = RiaApplication::instance();
    QString         text = app->commandLineParameterHelp();
    app->showFormattedTextInMessageBoxOrConsole( text );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHelpCommandLineFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "&Command Line Help" );
    actionToSetup->setIcon( QIcon( ":/HelpCircle.svg" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHelpSummaryCommandLineFeature::onActionTriggered( bool isChecked )
{
    disableModelChangeContribution();

    RiaApplication* app  = RiaApplication::instance();
    QString         text = RicSummaryPlotFeatureImpl::summaryPlotCommandLineHelpText();
    app->showFormattedTextInMessageBoxOrConsole( text );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHelpSummaryCommandLineFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "&Summary Command Line Help" );
    actionToSetup->setIcon( QIcon( ":/HelpCircle.svg" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHelpOpenUsersGuideFeature::onActionTriggered( bool isChecked )
{
    disableModelChangeContribution();

    QString usersGuideUrl = "https://resinsight.org/getting-started/overview/";
    RiaNetworkTools::openUrlWithErrorReporting( usersGuideUrl );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHelpOpenUsersGuideFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "&Users Guide" );
    actionToSetup->setIcon( QIcon( ":/HelpCircle.svg" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSearchHelpFeature::onActionTriggered( bool isChecked )
{
    disableModelChangeContribution();

    QString usersGuideUrl = "https://resinsight.org/getting-started/overview/";

    caf::PdmUiItem* uiItem = caf::SelectionManager::instance()->selectedItem();
    if ( uiItem && !uiItem->uiName().isEmpty() )
    {
        usersGuideUrl = "https://resinsight.org/search/?q=" + uiItem->uiName();
    }

    RiaNetworkTools::openUrlWithErrorReporting( usersGuideUrl );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSearchHelpFeature::setupActionLook( QAction* actionToSetup )
{
    caf::PdmUiItem* uiItem = caf::SelectionManager::instance()->selectedItem();

    if ( uiItem && !uiItem->uiName().isEmpty() )
    {
        actionToSetup->setText( "Search Help For: " + uiItem->uiName() );
    }
    else
    {
        actionToSetup->setText( "Search Help" );
    }
    actionToSetup->setIcon( QIcon( ":/HelpCircle.svg" ) );

    applyShortcutWithHintToAction( actionToSetup, QKeySequence::HelpContents );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSearchIssuesHelpFeature::onActionTriggered( bool isChecked )
{
    QString usersGuideUrl = "https://github.com/OPM/ResInsight/issues";
    RiaNetworkTools::openUrlWithErrorReporting( usersGuideUrl );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSearchIssuesHelpFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Search Issues" );
    actionToSetup->setIcon( QIcon( ":/HelpCircle.svg" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateNewIssueHelpFeature::onActionTriggered( bool isChecked )
{
    QString usersGuideUrl = "https://github.com/OPM/ResInsight/issues/new";
    RiaNetworkTools::openUrlWithErrorReporting( usersGuideUrl );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateNewIssueHelpFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create New Issue" );
    actionToSetup->setIcon( QIcon( ":/HelpCircle.svg" ) );
}
