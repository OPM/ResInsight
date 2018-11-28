/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RigHexIntersectionTools.h"
#include "QDateTime"
#include "QDebug"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RigHexIntersectionTools, planeHexCellIntersectionTest)
{
    cvf::Vec3d hexCorners[8];
    hexCorners[0] = cvf::Vec3d(0, 0, 0);
    hexCorners[1] = cvf::Vec3d(1, 0, 0);
    hexCorners[2] = cvf::Vec3d(0, 1, 0);
    hexCorners[3] = cvf::Vec3d(0, 0, 1);
    hexCorners[4] = cvf::Vec3d(0, 1, 1);
    hexCorners[5] = cvf::Vec3d(1, 1, 0);
    hexCorners[6] = cvf::Vec3d(1, 0, 1);
    hexCorners[7] = cvf::Vec3d(1, 1, 1);

    std::list<std::pair<cvf::Vec3d, cvf::Vec3d > > intersectionLineSegments;
    bool isCellIntersected = false;
    cvf::Plane fracturePlane;
    
    fracturePlane.setFromPointAndNormal(cvf::Vec3d(0.5, 0.5, 0.5), cvf::Vec3d(1, 0, 0) );
    isCellIntersected = RigHexIntersectionTools::planeHexCellIntersection(hexCorners, fracturePlane, intersectionLineSegments);
    EXPECT_TRUE(isCellIntersected);

    fracturePlane.setFromPointAndNormal(cvf::Vec3d(1.5, 1.5, 1.5), cvf::Vec3d(1, 0, 0));
    isCellIntersected = RigHexIntersectionTools::planeHexCellIntersection(hexCorners, fracturePlane, intersectionLineSegments);
    EXPECT_FALSE(isCellIntersected);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RigHexIntersectionTools, DISABLED_planeHexCellIntersectionPerformanceTest)
{
    cvf::Vec3d hexCorners[8];
    hexCorners[0] = cvf::Vec3d(0, 0, 0);
    hexCorners[1] = cvf::Vec3d(1, 0, 0);
    hexCorners[2] = cvf::Vec3d(0, 1, 0);
    hexCorners[3] = cvf::Vec3d(0, 0, 1);
    hexCorners[4] = cvf::Vec3d(0, 1, 1);
    hexCorners[5] = cvf::Vec3d(1, 1, 0);
    hexCorners[6] = cvf::Vec3d(1, 0, 1);
    hexCorners[7] = cvf::Vec3d(1, 1, 1);

    bool isCellIntersected = false;
    cvf::Plane fracturePlaneIntersecting;
    cvf::Plane fracturePlaneNotIntersecting;

    fracturePlaneNotIntersecting.setFromPointAndNormal(cvf::Vec3d(1.5, 1.5, 1.5), cvf::Vec3d(1, 0, 0));
    fracturePlaneIntersecting.setFromPointAndNormal(cvf::Vec3d(0.5, 0.5, 0.5), cvf::Vec3d(1, 0, 0));

    QTime timeTotal;
    timeTotal.start();

    for (int run = 0; run < 5; run++)
    {
        QTime timeLocal;
        timeLocal.start();

        for (int i = 0; i < 2000000; i++)
        {
            std::list<std::pair<cvf::Vec3d, cvf::Vec3d > > intersectionLineSegments;

            isCellIntersected = RigHexIntersectionTools::planeHexCellIntersection(hexCorners, fracturePlaneIntersecting, intersectionLineSegments);
            isCellIntersected = RigHexIntersectionTools::planeHexCellIntersection(hexCorners, fracturePlaneNotIntersecting, intersectionLineSegments);
        }

        qDebug() << "Time rim elapsed: " << timeLocal.elapsed();
    }

    qDebug() << "Time total elapsed: " << timeTotal.elapsed();
}

