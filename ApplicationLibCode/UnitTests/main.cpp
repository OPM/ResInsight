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

#include <QLocale>

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
    int result = RUN_ALL_TESTS();
    return result;
}
