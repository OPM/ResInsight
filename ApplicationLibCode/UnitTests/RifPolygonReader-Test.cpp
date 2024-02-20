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

#include "RifPolygonReader.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifPolygonReader, MultiplePolygonsInSameFile )
{
    std::string fileContent = R"(
# This is a comment
# This is a comment
58177.76 732.7 1643.6 
58260.83 732.8 1596.6 
57985.66 732.7 1542.0 
59601.45 732.4 3639.0 
59422.01 732.2 3639.0 
59793.41 732.2 3639.0 
999 999 999
# starting polyline 2
58260.83 732.8 1596.6 
57985.66 732.7 1542.0 
59601.45 732.4 3639.0 )";

    QString fileContentAsQString = QString::fromStdString( fileContent );
    QString errorMessage;

    auto polygons = RifPolygonReader::parseText( fileContentAsQString, &errorMessage );
    EXPECT_TRUE( polygons.size() == 2 );

    auto firstPolygon = polygons.front();
    EXPECT_TRUE( firstPolygon.size() == 6 );
    EXPECT_DOUBLE_EQ( firstPolygon.front().x(), 58177.76 );

    auto secondPolygon = polygons.back();
    EXPECT_TRUE( secondPolygon.size() == 3 );
    EXPECT_DOUBLE_EQ( secondPolygon.back().x(), 59601.45 );
}

TEST( RifPolygonReader, CsvImport )
{
    std::string fileContent = R"(
X_UTME,Y_UTMN,Z_TVDSS,POLY_ID
460536.5500488281,5934613.072753906,1652.97265625,0
460535.4345703125,5934678.220581055,1652.8203125,0
460534.3497314453,5934741.589111328,1652.672119140625,0
460534.5389404297,5934747.926269531,1652.6595458984375,0
460530.44287109375,5934829.321044922,1652.448974609375,0
460530.20642089844,5934842.320922852,1652.6920166015625,0
460535.5516357422,5934890.617675781,1653.71240234375,0
460548.05139160156,5935007.113769531,1656.166259765625,0
460550.3292236328,5935027.826538086,1656.641845703125,0
)";

    QString fileContentAsQString = QString::fromStdString( fileContent );
    QString errorMessage;

    auto polygons = RifPolygonReader::parseTextCsv( fileContentAsQString, &errorMessage );
    EXPECT_TRUE( polygons.size() == 1 );

    auto firstPolygon = polygons.front();
    EXPECT_TRUE( firstPolygon.size() == 9 );
    EXPECT_DOUBLE_EQ( firstPolygon.back().z(), -1656.641845703125 );
}
