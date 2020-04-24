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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHelpAboutFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHelpAboutFeature::onActionTriggered( bool isChecked )
{
    this->disableModelChangeContribution();

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
                txt += " and supported";
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

#ifdef ENABLE_GRPC
    RiaGrpcServer* grpcServer = RiaApplication::instance()->grpcServer();
    if ( grpcServer && grpcServer->isRunning() )
    {
        dlg.addVersionEntry( " ",
                             QString( "   Python Script Server available and running at port %1" )
                                 .arg( grpcServer->portNumber() ) );
    }
    else
    {
        dlg.addVersionEntry( " ", QString( "   Python Script Server available but currently disabled in preferences" ) );
    }
#else
    dlg.addVersionEntry( " ", "   gRPC disabled" );
#endif

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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHelpCommandLineFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHelpCommandLineFeature::onActionTriggered( bool isChecked )
{
    this->disableModelChangeContribution();

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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHelpSummaryCommandLineFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHelpSummaryCommandLineFeature::onActionTriggered( bool isChecked )
{
    this->disableModelChangeContribution();

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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHelpOpenUsersGuideFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHelpOpenUsersGuideFeature::onActionTriggered( bool isChecked )
{
    this->disableModelChangeContribution();

    QString usersGuideUrl = "https://resinsight.org/getting-started/overview/";

    if ( !QDesktopServices::openUrl( usersGuideUrl ) )
    {
        QErrorMessage* errorHandler = QErrorMessage::qtHandler();
        errorHandler->showMessage( "Failed open browser with the following url\n\n" + usersGuideUrl );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHelpOpenUsersGuideFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "&Users Guide" );

    // applyShortcutWithHintToAction( actionToSetup, QKeySequence::HelpContents );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSearchHelpFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSearchHelpFeature::onActionTriggered( bool isChecked )
{
    this->disableModelChangeContribution();

    QString usersGuideUrl = "https://resinsight.org/getting-started/overview/";

    caf::PdmUiItem* uiItem = caf::SelectionManager::instance()->selectedItem();
    if ( uiItem && !uiItem->uiName().isEmpty() )
    {
        usersGuideUrl = "https://resinsight.org/search/?q=" + uiItem->uiName();
    }

    if ( !QDesktopServices::openUrl( usersGuideUrl ) )
    {
        QErrorMessage* errorHandler = QErrorMessage::qtHandler();
        errorHandler->showMessage( "Failed open browser with the following url\n\n" + usersGuideUrl );
    }
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

    applyShortcutWithHintToAction( actionToSetup, QKeySequence::HelpContents );
}
