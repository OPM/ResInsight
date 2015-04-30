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

#include "gtest/gtest.h"
#include "RifOdbReader.h"
#include "RigGeoMechCaseData.h"

#include <vector>
#include <string>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OdbReaderTest, BasicTests)
{
    std::cout << TEST_FILE << std::endl;
    std::cout << std::endl;
    cvf::ref<RifOdbReader> reader = new RifOdbReader;
    cvf::ref<RigGeoMechCaseData> femCase = new RigGeoMechCaseData;
    cvf::ref<RigFemPartCollection> femData = femCase->femParts();
    reader->readFemParts(TEST_FILE, femData.p());
    EXPECT_EQ(1, femData->partCount());
    EXPECT_EQ(4320, femData->part(0)->elementCount());
    EXPECT_EQ(HEX8, femData->part(0)->elementType(0));

	cvf::ref<RifOdbReader> reader2 = new RifOdbReader;
	EXPECT_EQ(true, reader2->openFile(TEST_FILE));
	EXPECT_EQ(true, reader2->stepNames().size() == 1);

	std::vector<std::string> steps = reader2->stepNames();
	EXPECT_EQ(true, steps.at(0).find("Date_20100930") >= 0);
	EXPECT_EQ(2, reader2->frameTimes(0).size());
	EXPECT_EQ(1.0, reader2->frameTimes(0)[1]);

	std::map<std::string, std::vector<std::string> > scalarNodeFieldsMap = reader2->scalarNodeFieldAndComponentNames();
	EXPECT_EQ(3, scalarNodeFieldsMap.size());
	
	std::map<std::string, std::vector<std::string> > scalarElementNodeFieldsMap = reader2->scalarElementNodeFieldAndComponentNames();
	EXPECT_EQ(0, scalarElementNodeFieldsMap.size());
	
	std::map<std::string, std::vector<std::string> > scalarIntegrationPointFieldsMap = reader2->scalarIntegrationPointFieldAndComponentNames();
	EXPECT_EQ(6, scalarIntegrationPointFieldsMap.size());

	std::vector<float> displacementValues;
	reader2->readScalarNodeField("U", "U2", 0, 0, 1, &displacementValues);
	EXPECT_EQ(5168, displacementValues.size());

	std::vector<cvf::Vec3f> displacements;
	reader2->readDisplacements(0, 0, 1, &displacements);
	EXPECT_EQ(5168, displacements.size());
	EXPECT_FLOAT_EQ(0.047638997, displacements[1].y());
}


