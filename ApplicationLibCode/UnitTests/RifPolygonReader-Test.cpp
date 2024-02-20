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
