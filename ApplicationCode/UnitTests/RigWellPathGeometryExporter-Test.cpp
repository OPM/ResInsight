/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RigWellPathGeometryExporter.h"

#include "RigWellPath.h"

#include "cvfVector3.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigWellPathGeometryExporter, VerticalPath )
{
    double x = 457313.52;
    double y = 7320309.71;

    // Make a vertical well
    RigWellPath rigWellPath;

    std::vector<double> inputMds = {0.0, 1032.14, 1548.2, 2580.34, 2619.82, 2777.73, 2790.57, 2810.34};

    for ( double md : inputMds )
    {
        rigWellPath.m_measuredDepths.push_back( md );
        rigWellPath.m_wellPathPoints.push_back( cvf::Vec3d( x, y, -md ) );
    }

    double              mdStepSize = 5.0;
    double              rkbOffset  = 0.0;
    std::vector<double> xValues;
    std::vector<double> yValues;
    std::vector<double> tvdValues;
    std::vector<double> mdValues;
    RigWellPathGeometryExporter::exportWellPathGeometry( &rigWellPath, mdStepSize, rkbOffset, xValues, yValues, tvdValues, mdValues );

    double firstMd               = inputMds.front();
    double lastMd                = inputMds.back();
    size_t expectedNumDataPoints = static_cast<size_t>( std::abs( lastMd - firstMd ) / mdStepSize ) + 2;
    EXPECT_EQ( expectedNumDataPoints, tvdValues.size() );
    EXPECT_EQ( expectedNumDataPoints, mdValues.size() );

    // The change in MD should always be bigger than change in TVD (or very close)
    for ( size_t i = 1; i < tvdValues.size(); i++ )
    {
        double changeMd  = mdValues[i] - mdValues[i - 1];
        double changeTvd = tvdValues[i] - tvdValues[i - 1];
        double diff      = std::abs( changeTvd - changeMd );
        ASSERT_TRUE( changeMd > changeTvd || diff < 0.0000000001 );
    }
}
