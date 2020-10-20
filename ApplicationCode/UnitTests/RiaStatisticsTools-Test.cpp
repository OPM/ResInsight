/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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

#include "RiaStatisticsTools.h"

#include <QDebug>
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaStatisticsTools, NoCorrelation )
{
    const int           N = 1000;
    std::vector<double> a, b;
    a.reserve( N );
    b.reserve( N );
    for ( int i = 0; i < N; ++i )
    {
        a.push_back( (double)i );
    }
    for ( int i = 0; i < N; ++i )
    {
        b.push_back( (double)std::rand() );
    }
    double correlation = RiaStatisticsTools::pearsonCorrelation( a, b );
    EXPECT_LE( correlation, 0.25 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaStatisticsTools, FullCorrelation )
{
    const int           N = 1000;
    std::vector<double> a, b;
    a.reserve( N );
    b.reserve( N );
    for ( int i = 0; i < N; ++i )
    {
        a.push_back( (double)i );
    }
    for ( int i = 0; i < N; ++i )
    {
        b.push_back( i * 2.0 + 1.0 );
    }
    double correlation = RiaStatisticsTools::pearsonCorrelation( a, b );
    EXPECT_NEAR( correlation, 1.0, 1.0e-2 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaStatisticsTools, NegativeCorrelation )
{
    const int           N = 1000;
    std::vector<double> a, b;
    a.reserve( N );
    b.reserve( N );
    for ( int i = 0; i < N; ++i )
    {
        a.push_back( (double)i );
    }
    for ( int i = 0; i < N; ++i )
    {
        b.push_back( i * -2.0 + 1.0 );
    }
    double correlation = RiaStatisticsTools::pearsonCorrelation( a, b );
    EXPECT_NEAR( correlation, -1.0, 1.0e-2 );
}
