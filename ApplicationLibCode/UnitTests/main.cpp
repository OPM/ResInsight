/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RiaConsoleApplication.h"
#include "RiaQuantityInfoTools.h"
#include "RiaRegressionTestRunner.h"

#include "RimProject.h"

#include <QLocale>
#include <QStringList>

//--------------------------------------------------------------------------------------------------
/// The project is a global object shared by all tests. A test leaving data behind in the project
/// makes the outcome of the tests running after it depend on the test order, and the test order is
/// not the same on all platforms.
///
/// Fail the test that leaves data behind, and close the project so the tests after it are unaffected.
//--------------------------------------------------------------------------------------------------
class RiaProjectIsolationListener : public testing::EmptyTestEventListener
{
private:
    void OnTestEnd( const testing::TestInfo& ) override
    {
        RimProject* project = RimProject::current();
        if ( !project ) return;

        const QString leftovers = leftoverDescription( project );
        if ( leftovers.isEmpty() ) return;

        RiaApplication::instance()->closeProject();

        ADD_FAILURE() << "The test left data behind in the shared project: " << leftovers.toStdString()
                      << ". Call RiaApplication::instance()->closeProject() before the test completes.";
    }

    static QString leftoverDescription( RimProject* project )
    {
        QStringList leftovers;

        auto appendCount = [&leftovers]( const QString& description, size_t count )
        {
            if ( count > 0 ) leftovers.append( QString( "%1 %2" ).arg( count ).arg( description ) );
        };

        appendCount( "grid case(s)", project->allGridCases().size() );
        appendCount( "summary case(s)", project->allSummaryCases().size() );
        appendCount( "summary ensemble(s)", project->summaryEnsembles().size() );
        appendCount( "well path(s)", project->allWellPaths().size() );
        appendCount( "view(s)", project->allViews().size() );

        if ( !project->fileName().isEmpty() ) leftovers.append( "a project file name" );

        return leftovers.join( ", " );
    }
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int main( int argc, char** argv )
{
    // Create feature manager before the application object is created
    RiaRegressionTestRunner::createSingleton();
    RiaQuantityInfoTools::initializeSummaryKeywords();

    RiaApplication* app = new RiaConsoleApplication( argc, argv );
    app->initialize();

    QLocale::setDefault( QLocale( QLocale::English, QLocale::UnitedStates ) );
    setlocale( LC_NUMERIC, "C" );

    testing::InitGoogleTest( &argc, argv );

    // OnTestEnd is dispatched in reverse order of appending, so the listener appended last is the
    // first to see the end of a test. The failure it reports is then part of the printed test result.
    testing::UnitTest::GetInstance()->listeners().Append( new RiaProjectIsolationListener );

    int result = RUN_ALL_TESTS();
    return result;
}
