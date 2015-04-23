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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OdbReaderTest, BasicTests)
{
    cvf::ref<RifOdbReader> reader = new RifOdbReader;
    cvf::ref<RigGeoMechCaseData> femData = new RigGeoMechCaseData;
    reader->open("C:\\pfRoot\\jjsOnJacobpcCsdep\\User\\Sigurd\\OdbApiExperiments\\viewer_tutorial.odb", femData.p());

    EXPECT_EQ(1, femData->partCount());
    EXPECT_EQ(140, femData->part(0)->elementCount());
    EXPECT_EQ(CAX4, femData->part(0)->elementType(0));

}


