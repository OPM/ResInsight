/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025- Equinor ASA
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

#include "RifRmsWellPathReader.h"

#include "RiaTestDataDirectory.h"

static const QString TEST_DATA_DIRECTORY = QString( "%1/RifRmsWellPathReader/" ).arg( TEST_DATA_DIR );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RifRmsWellPathReader, ReadFromFile )
{
    QString filePath = TEST_DATA_DIRECTORY + "55_33-1.rmswell";
    auto    wellData = RifRmsWellPathReader::readWellData( filePath );
    EXPECT_EQ( wellData.m_name, QString( "55_33-1" ) );

    auto wellPath = wellData.m_wellPathGeometry;
    ASSERT_TRUE( wellPath.notNull() );

    EXPECT_EQ( 4647u, wellPath->wellPathPoints().size() );
    EXPECT_EQ( 4647u, wellPath->measuredDepths().size() );
}
