/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "gtest/gtest.h"

#include "RiaGuiApplication.h"
#include "RiaQuantityInfoTools.h"
#include "RiaRegressionTestRunner.h"

#include "RicFeatureExecutionRunner.h"

#include "cafCmdFeatureManager.h"
#include "cafPdmDefaultObjectFactory.h"

#include "cvfAssert.h"

#include <QApplication>
#include <QLocale>

#include <string>

#ifdef WIN32
#include <windows.h>

#include <cstdlib>
#endif

//--------------------------------------------------------------------------------------------------
/// Entry point for the command-feature test executable.
///
/// Unlike ResInsight-tests (which boots a headless RiaConsoleApplication), the feature sweep
/// executes Ric*Feature commands that call RiaGuiApplication::instance(). We therefore boot a full
/// RiaGuiApplication. On CI this is run with QT_QPA_PLATFORM=offscreen so no display is required.
//--------------------------------------------------------------------------------------------------
int main( int argc, char** argv )
{
    // Default to the offscreen platform plugin unless the environment already selected one. This
    // lets the executable run on headless CI without an X server / display.
    if ( qEnvironmentVariableIsEmpty( "QT_QPA_PLATFORM" ) )
    {
        qputenv( "QT_QPA_PLATFORM", "offscreen" );
    }

    // Route failed CVF asserts to the console instead of the native "Assertion Failed" message box.
    // That message box is a native (non-Qt) modal dialog the sweep watchdog cannot close, which would
    // hang the run when a feature is executed with a selection that violates its preconditions.
    cvf::Assert::setReportMode( cvf::Assert::CONSOLE );

#ifdef WIN32
    // Keep the process from popping Windows Error Reporting / CRT abort dialogs during the sweep so a
    // crashing or asserting feature fails deterministically instead of blocking on a dialog.
    SetErrorMode( SEM_NOGPFAULTERRORBOX | SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX );
    _set_abort_behavior( 0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT );
#endif

    // Create the singletons the features depend on before the application object is created, in the
    // same order as RiaMainTools::initializeSingletons().
    caf::CmdFeatureManager::createSingleton();
    RiaRegressionTestRunner::createSingleton();
    caf::PdmDefaultObjectFactory::createSingleton();
    RiaQuantityInfoTools::initializeSummaryKeywords();

    RiaGuiApplication app( argc, argv );
    app.initialize();

    // Enable the regression-test code paths that suppress modal error dialogs, so features that
    // would otherwise pop a QMessageBox during the sweep log to the console instead of blocking.
    RiaRegressionTestRunner::instance()->setRunningRegressionTests( true );

    QLocale::setDefault( QLocale( QLocale::English, QLocale::UnitedStates ) );
    setlocale( LC_NUMERIC, "C" );

    // Child mode for the subprocess sweep: execute a single feature and exit without running any
    // gtest cases. The parent (RicFeatureSubprocessSweep-Test.cpp) spawns one child per feature, so a
    // feature that aborts on a failed CVF_ASSERT/CAF_ASSERT, or hangs, only takes down its own
    // process and is attributed to that feature id.
    const std::string execSwitch = RicFeatureExecutionRunner::featureExecSwitch;
    for ( int i = 1; i < argc; ++i )
    {
        const std::string arg = argv[i];
        if ( arg.rfind( execSwitch, 0 ) == 0 )
        {
            // Unbuffered stdout: a failing CAF_ASSERT prints the file and line with std::printf and
            // then calls std::abort(), which does not flush. Buffered output would drop exactly the
            // message identifying where the child died.
            setvbuf( stdout, nullptr, _IONBF, 0 );

            const std::string commandId = arg.substr( execSwitch.size() );
            const bool        ok        = RicFeatureExecutionRunner::executeFeature( commandId );

            // Close the project before RiaGuiApplication is destroyed. Tearing down a live project
            // (cases, views) during application destruction crashes, which would make every child
            // look like a crashing feature.
            RicFeatureExecutionRunner::releaseModel();

            return ok ? 0 : 1;
        }
    }

    testing::InitGoogleTest( &argc, argv );
    int result = RUN_ALL_TESTS();
    return result;
}
