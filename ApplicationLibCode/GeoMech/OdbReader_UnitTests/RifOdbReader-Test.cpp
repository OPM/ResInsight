/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RifOdbReader.h"
#include "RigFemPartCollection.h"
#include "RigGeoMechCaseData.h"
#include "gtest/gtest.h"

#include "cvfDebugTimer.h"

#include <string>
#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( OdbReaderTest, BasicTests )
{
    std::cout << TEST_FILE << std::endl;
    std::cout << std::endl;
    cvf::ref<RifOdbReader>         reader  = new RifOdbReader;
    cvf::ref<RigGeoMechCaseData>   femCase = new RigGeoMechCaseData( "" );
    cvf::ref<RigFemPartCollection> femData = femCase->femParts();

    cvf::DebugTimer timer( "DebugTimer" );
    timer.reportTime();

    reader->openFile( TEST_FILE );
    reader->readFemParts( femData.p() );

    EXPECT_EQ( 1, femData->partCount() );
    EXPECT_EQ( 4320, femData->part( 0 )->elementCount() );
    EXPECT_EQ( HEX8, femData->part( 0 )->elementType( 0 ) );

    EXPECT_EQ( true, reader->stepNames().size() == 1 );

    std::vector<std::string> steps = reader->stepNames();
    EXPECT_EQ( true, steps.at( 0 ).find( "Date_20100930" ) >= 0 );
    EXPECT_EQ( 2, reader->frameTimes( 0 ).size() );
    EXPECT_EQ( 1.0, reader->frameTimes( 0 )[1] );

    std::map<std::string, std::vector<std::string>> scalarNodeFieldsMap = reader->scalarNodeFieldAndComponentNames();
    EXPECT_EQ( 3, scalarNodeFieldsMap.size() );

    std::map<std::string, std::vector<std::string>> scalarElementNodeFieldsMap =
        reader->scalarElementNodeFieldAndComponentNames();
    EXPECT_EQ( 0, scalarElementNodeFieldsMap.size() );

    std::map<std::string, std::vector<std::string>> scalarIntegrationPointFieldsMap =
        reader->scalarIntegrationPointFieldAndComponentNames();
    EXPECT_EQ( 6, scalarIntegrationPointFieldsMap.size() );

    std::vector<float> displacementValues;
    reader->readScalarNodeField( "U", "U2", 0, 0, 1, &displacementValues );
    EXPECT_EQ( 5168, displacementValues.size() );

    std::vector<float> integrationPointS22;
    timer.restart();
    reader->readScalarIntegrationPointField( "S", "S22", 0, 0, 1, &integrationPointS22 );
    timer.reportLapTime( "Read S/S22" );
    timer.restart();
    reader->readScalarIntegrationPointField( "S", "S22", 0, 0, 1, &integrationPointS22 );
    timer.reportLapTime( "Read S/S22 2nd time" );
    timer.restart();
    reader->readScalarIntegrationPointField( "S", "S22", 0, 0, 1, &integrationPointS22 );
    timer.reportLapTime( "Read S/S22 3rd time" );
    EXPECT_EQ( 34560, integrationPointS22.size() );
    EXPECT_FLOAT_EQ( -1921117.3, integrationPointS22[0] );
    EXPECT_FLOAT_EQ( -1408592.5, integrationPointS22[1] );
    EXPECT_FLOAT_EQ( -1345666.9, integrationPointS22[2] );

    std::vector<float> elementNodeS11;
    reader->readScalarElementNodeField( "S", "S11", 0, 0, 1, &elementNodeS11 );
    EXPECT_EQ( 34560, elementNodeS11.size() );
    EXPECT_FLOAT_EQ( -2074357.3, elementNodeS11[0] );
    EXPECT_FLOAT_EQ( -1353137.5, elementNodeS11[1] );
    EXPECT_FLOAT_EQ( -1144559.4, elementNodeS11[2] );

    std::vector<float> integrationPointE33;
    reader->readScalarIntegrationPointField( "E", "E33", 0, 0, 1, &integrationPointE33 );
    EXPECT_EQ( 34560, integrationPointE33.size() );

    std::vector<float> integrationPointTEMP;
    reader->readScalarIntegrationPointField( "TEMP", "", 0, 0, 1, &integrationPointTEMP );
    EXPECT_EQ( 34560, integrationPointTEMP.size() );

    std::vector<cvf::Vec3f> displacements;
    reader->readDisplacements( 0, 0, 1, &displacements );
    EXPECT_EQ( 5168, displacements.size() );
    EXPECT_FLOAT_EQ( 0.047638997, displacements[1].y() );
    EXPECT_FLOAT_EQ( -0.0036307564, displacements[6].x() );
    EXPECT_FLOAT_EQ( 0.065709047, displacements[6].y() );
    EXPECT_FLOAT_EQ( -0.059760433, displacements[6].z() );

    std::vector<float> porValues;
    reader->readScalarNodeField( "POR", "", 0, 0, 1, &porValues );
    EXPECT_EQ( 5168, porValues.size() );

    std::vector<float> voidrValues;
    reader->readScalarIntegrationPointField( "VOIDR", "", 0, 0, 0, &voidrValues );
    EXPECT_EQ( 34560, voidrValues.size() );
    EXPECT_FLOAT_EQ( 0.22864963, voidrValues[0] );
    EXPECT_FLOAT_EQ( 0.23406270, voidrValues[1] );
    EXPECT_FLOAT_EQ( 0.24549910, voidrValues[2] );

    timer.restart();
    reader->readScalarIntegrationPointField( "S", "S22", 0, 0, 1, &integrationPointS22 );
    timer.reportLapTime( "Read S/S22 final time" );
}
