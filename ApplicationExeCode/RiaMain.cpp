/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RiaArgumentParser.h"
#include "RiaMainTools.h"
#include "RiaPreferences.h"

#ifdef ENABLE_GRPC
#include "RiaGrpcConsoleApplication.h"
#include "RiaGrpcGuiApplication.h"
#else
#include "RiaConsoleApplication.h"
#include "RiaGuiApplication.h"
#endif

#include "cafUiAppearanceSettings.h"

#include "cvfProgramOptions.h"
#include "cvfqtUtils.h"

#include <QFile>

#ifndef WIN32
#include <sys/types.h>
#include <unistd.h>
#endif

#include <signal.h>

void manageSegFailure( int signalCode );

RiaApplication* createApplication( int& argc, char* argv[] )
{
    for ( int i = 1; i < argc; ++i )
    {
        if ( !qstrcmp( argv[i], "--console" ) || !qstrcmp( argv[i], "--unittest" ) )
        {
#ifdef ENABLE_GRPC
            return new RiaGrpcConsoleApplication( argc, argv );
#else
            return new RiaConsoleApplication( argc, argv );
#endif
        }
    }
#ifdef ENABLE_GRPC
    return new RiaGrpcGuiApplication( argc, argv );
#else
    return new RiaGuiApplication( argc, argv );
#endif
}

int main( int argc, char* argv[] )
{
#ifndef WIN32
    // From Qt 5.3 and onwards Qt has a mechanism for checking this automatically
    // But it only checks user id not group id, so better to do it ourselves.
    if ( getuid() != geteuid() || getgid() != getegid() )
    {
        std::cerr << "FATAL: The application binary appears to be running setuid or setgid, this is a security hole."
                  << std::endl;
        return 1;
    }
#endif

    // The Qt::AA_ShareOpenGLContexts setting is needed when we have multiple viz widgets in flight
    // and we have a setup where these widgets belong to different top-level windows, or end up
    // belonging to different top-level windows through re-parenting.
    // See test application QtTestBenchOpenGLWidget
    QApplication::setAttribute( Qt::AA_ShareOpenGLContexts );

    // Create feature manager before the application object is created
    RiaMainTools::initializeSingletons();

    // https://www.w3.org/wiki/CSS/Properties/color/keywords
    caf::UiAppearanceSettings::instance()->setAutoValueEditorColor( "moccasin" );

    std::unique_ptr<RiaApplication> app( createApplication( argc, argv ) );

    cvf::ProgramOptions progOpt;
    bool                result = RiaArgumentParser::parseArguments( &progOpt );

    const cvf::String usageText = progOpt.usageText( 110, 30 );
    app->initialize();
    app->setCommandLineHelpText( cvfqt::Utils::toQString( usageText ) );

    if ( !result )
    {
        std::vector<cvf::String> unknownOptions = progOpt.unknownOptions();
        QString                  unknownOptionsText;
        for ( cvf::String option : unknownOptions )
        {
            unknownOptionsText += QString( "\tUnknown option: %1\n" ).arg( cvfqt::Utils::toQString( option ) );
        }

        app->showFormattedTextInMessageBoxOrConsole(
            "ERROR: Unknown command line options detected ! \n" + unknownOptionsText + "\n\n" +
            "The current command line options in ResInsight are:\n" + app->commandLineParameterHelp() );

        if ( dynamic_cast<RiaGuiApplication*>( app.get() ) == nullptr )
        {
            return 1;
        }
    }

    QLocale::setDefault( QLocale( QLocale::English, QLocale::UnitedStates ) );
    setlocale( LC_NUMERIC, "C" );

    // Set up signal handlers
    if ( RiaPreferences::current()->loggerTrapSignalAndFlush() )
    {
        signal( SIGINT, manageSegFailure );
        signal( SIGILL, manageSegFailure );
        signal( SIGFPE, manageSegFailure );
        signal( SIGSEGV, manageSegFailure );
        signal( SIGTERM, manageSegFailure );
        signal( SIGABRT, manageSegFailure );
    }

    // Handle the command line arguments.
    // Todo: Move to a one-shot timer, delaying the execution until we are inside the event loop.
    // The complete handling of the resulting ApplicationStatus must be moved along.
    // The reason for this is: deleteLater() does not work outside the event loop
    // Make execution of command line stuff operate in identical conditions as interactive operation.

    RiaApplication::ApplicationStatus status = app->handleArguments( &progOpt );

    if ( status == RiaApplication::ApplicationStatus::EXIT_COMPLETED )
    {
        // Make sure project is closed to avoid assert and crash in destruction of widgets
        app->closeProject();

        app.reset();
        RiaMainTools::releaseSingletonAndFactoryObjects();

        return 0;
    }
    else if ( status == RiaApplication::ApplicationStatus::EXIT_WITH_ERROR )
    {
        // Make sure project is closed to avoid assert and crash in destruction of widgets
        app->closeProject();

        return 2;
    }
    else if ( status == RiaApplication::ApplicationStatus::KEEP_GOING )
    {
        int exitCode = 0;
        try
        {
#ifdef ENABLE_GRPC
            auto grpcInterface = dynamic_cast<RiaGrpcApplicationInterface*>( app.get() );
            if ( grpcInterface && grpcInterface->initializeGrpcServer( progOpt ) )
            {
                grpcInterface->launchGrpcServer();

                if ( cvf::Option o = progOpt.option( "portnumberfile" ) )
                {
                    if ( o.valueCount() == 1 )
                    {
                        int     portNumber = grpcInterface->portNumber();
                        QString fileName   = QString::fromStdString( o.value( 0 ).toStdString() );

                        // Write port number to the file given file.
                        // Temp file is used to avoid incomplete reads.
                        QString tempFilePath = fileName + ".tmp";
                        QFile   file( tempFilePath );
                        if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) )
                        {
                            QTextStream out( &file );
                            out << portNumber << "\n";
                        }
                        file.close();

                        QFile::rename( tempFilePath, fileName );
                    }
                }
            }
#endif
            exitCode = QCoreApplication::instance()->exec();
        }
        catch ( std::exception& exep )
        {
            std::cout << "A standard c++ exception that terminated ResInsight caught in RiaMain.cpp: " << exep.what()
                      << std::endl;
            throw;
        }
        catch ( ... )
        {
            std::cout << "An unknown exception that terminated ResInsight caught in RiaMain.cpp.  " << std::endl;
            throw;
        }

        app.reset();
        RiaMainTools::releaseSingletonAndFactoryObjects();

        return exitCode;
    }

    CVF_ASSERT( false && "Unknown ApplicationStatus" );
    return -1;
}
