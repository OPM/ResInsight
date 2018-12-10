/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Statoil ASA
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

#include "RifReaderMockModel.h"

#include "RigEclipseCaseData.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigGridManager.h"
#include "RigMainGrid.h"
#include "RigWellPath.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST(RigEclipseWellLogExtractor, ShortWellPathInsideOneCell)
{
    cvf::ref<RigEclipseCaseData> reservoir = new RigEclipseCaseData(nullptr);

    {
        cvf::ref<RifReaderMockModel> mockFileInterface = new RifReaderMockModel;

        mockFileInterface->setWorldCoordinates(cvf::Vec3d(10, 10, 10), cvf::Vec3d(20, 20, 20));
        mockFileInterface->setGridPointDimensions(cvf::Vec3st(4, 5, 6));
        mockFileInterface->enableWellData(false);

        mockFileInterface->open("", reservoir.p());

        reservoir->mainGrid()->computeCachedData();
    }

    auto cells = reservoir->mainGrid()->globalCellArray();
    EXPECT_FALSE(cells.empty());

    auto firstCell = reservoir->mainGrid()->globalCellArray()[0];
    auto center    = firstCell.center();

    cvf::ref<RigWellPath> wellPathGeometry = new RigWellPath;
    {
        std::vector<cvf::Vec3d> wellPathPoints;
        std::vector<double>     mdValues;

        {
            double offset = 0.0;
            wellPathPoints.push_back(center);
            mdValues.push_back(offset);
        }

        {
            double offset = 0.1;
            wellPathPoints.push_back(center + cvf::Vec3d(0, 0, offset));
            mdValues.push_back(offset);
        }

        wellPathGeometry->m_wellPathPoints = wellPathPoints;
        wellPathGeometry->m_measuredDepths = mdValues;
    }

    cvf::ref<RigEclipseWellLogExtractor> e = new RigEclipseWellLogExtractor(reservoir.p(), wellPathGeometry.p(), "");

    auto intersections = e->cellIntersectionInfosAlongWellPath();
    EXPECT_FALSE(intersections.empty());
}
