/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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

#include "RifOsduWellPathReader.h"

#include "RigWellPath.h"

#include "cvfObject.h"

TEST( RifOsduWellPathReader, ParseCsv )
{
    std::string fileContent = R"(
MD,TVD,AZIMUTH,INCLINATION,X,Y,GODLEG_SEVERITY,DX,DY,UWI,WELLBORE,CRS,EPSG_CODE
0.0,0.0,1e-10,0.0,68.8767382,33.1345775,0.0,0.0,0.0,WELL NAME #1,WELL NAME #1,WGS84,4326
271.49,271.49,1e-10,0.0,68.8767382,33.1345775,0.0,0.0,0.0,WELL NAME #1,WELL NAME #1,WGS84,4326
306.29,306.29,272.01823962,0.08968724,68.8767375,33.1345774,0.24,-0.06,-0.01,WELL NAME #1,WELL NAME #1,WGS84,4326
336.29,336.29,274.30140794,0.15846019,68.8767368,33.1345775,0.22,-0.12,0.0,WELL NAME #1,WELL NAME #1,WGS84,4326
366.29,366.29,278.8234303,0.09299958,68.8767359,33.1345775,0.23,-0.2,0.0,WELL NAME #1,WELL NAME #1,WGS84,4326
396.29,396.29,272.75661039,0.10458399,68.8767352,33.1345775,0.11,-0.26,0.0,WELL NAME #1,WELL NAME #1,WGS84,4326
426.56,426.29,268.84493373,0.12982945,68.8767347,33.1345775,0.24,-0.3,0.0,WELL NAME #1,WELL NAME #1,WGS84,4326
456.29,456.29,247.08294017,0.24449049,68.8767336,33.1345774,0.58,-0.39,-0.01,WELL NAME #1,WELL NAME #1,WGS84,4326
486.29,486.28,228.45433687,1.77934187,68.876729,33.1345746,1.0,-0.78,-0.33,WELL NAME #1,WELL NAME #1,WGS84,4326
516.29,516.25,234.24032672,3.25969626,68.8767165,33.1345672,1.41,-1.85,-1.15,WELL NAME #1,WELL NAME #1,WGS84,4326
546.29,546.17,235.37605057,5.34499442,68.8766954,33.1345561,3.39,-3.64,-2.39,WELL NAME #1,WELL NAME #1,WGS84,4326
576.29,575.97,233.86086873,7.54998951,68.8766625,33.1345381,1.98,-6.44,-4.4,WELL NAME #1,WELL NAME #1,WGS84,4326
)";

    QString fileContentAsQString = QString::fromStdString( fileContent );

    auto [wellPath, errorMessage] = RifOsduWellPathReader::parseCsv( fileContentAsQString );
    EXPECT_TRUE( wellPath.notNull() );

    EXPECT_EQ( 12u, wellPath->wellPathPoints().size() );
    EXPECT_EQ( 12u, wellPath->measuredDepths().size() );

    cvf::Vec3d point = wellPath->wellPathPoints()[6];
    EXPECT_DOUBLE_EQ( 68.8767347, point.x() );
    EXPECT_DOUBLE_EQ( 33.1345775, point.y() );
    EXPECT_DOUBLE_EQ( -426.29, point.z() );

    EXPECT_DOUBLE_EQ( 426.56, wellPath->measuredDepths()[6] );
}
