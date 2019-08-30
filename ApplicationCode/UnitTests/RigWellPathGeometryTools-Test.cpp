/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RigWellPathGeometryTools.h"

#include <vector>

#define TOLERANCE 1.0e-7

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST(RigWellPathGeometryTools, VerticalPath)
{
    std::vector<double> mdValues  = {100, 500, 1000};
    std::vector<double> tvdValues = {100, 500, 1000};
    std::vector<double> fullTVDValues = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
    std::vector<int> expectedSegmentIndices = {0, 0, 0, 0, 1, 1, 1, 1, 1, 2};
    std::vector<int> segmentIndices = RigWellPathGeometryTools::findSegmentIndices(mdValues, tvdValues, fullTVDValues);
    EXPECT_EQ(expectedSegmentIndices.size(), segmentIndices.size());
    for (size_t i = 0; i < expectedSegmentIndices.size(); ++i)
    {
        EXPECT_EQ(expectedSegmentIndices[i], segmentIndices[i]);
    }

    std::vector<double> fullMDValues  = RigWellPathGeometryTools::interpolateMdFromTvd(mdValues, tvdValues, fullTVDValues);

    EXPECT_EQ(fullTVDValues.size(), fullMDValues.size());
    for (size_t i = 0; i < fullTVDValues.size(); ++i)
    {
        EXPECT_NEAR(fullTVDValues[i], fullMDValues[i], TOLERANCE);
    }
}

TEST(RigWellPathGeometryTools, LinearPath)
{
    std::vector<double> mdValues = {100, 500, 1000};
    std::vector<double> tvdValues = {50, 250, 500};
    std::vector<double> fullTVDValues = {50, 100, 150, 200, 250, 300, 350, 400, 450, 500};

    std::vector<int> expectedSegmentIndices = {0, 0, 0, 0, 1, 1, 1, 1, 1, 2};
    std::vector<int> segmentIndices = RigWellPathGeometryTools::findSegmentIndices(mdValues, tvdValues, fullTVDValues);
    EXPECT_EQ(expectedSegmentIndices.size(), segmentIndices.size());
    for (size_t i = 0; i < expectedSegmentIndices.size(); ++i)
    {
        EXPECT_EQ(expectedSegmentIndices[i], segmentIndices[i]);
    }


    std::vector<double> fullMDValues = RigWellPathGeometryTools::interpolateMdFromTvd(mdValues, tvdValues, fullTVDValues);

    EXPECT_EQ(fullTVDValues.size(), fullMDValues.size());
    for (size_t i = 0; i < fullTVDValues.size(); ++i)
    {
        EXPECT_NEAR(2.0*fullTVDValues[i], fullMDValues[i], TOLERANCE);
    }
}

double quadraticFunction(double x)
{
    return 0.0015 * std::pow(x, 2) - 0.25 * x + 100;
}

TEST(RigWellPathGeometryTools, QuadraticPath)
{
    std::vector<double> mdValues = {100, 300, 600, 1000};
    std::vector<double> tvdValues;
    for (double md : mdValues)
    {
        tvdValues.push_back(quadraticFunction(md));
    }
    std::vector<double> fullMDValues = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
    std::vector<double> fullTvdValues;
    for (double md : fullMDValues)
    {
        fullTvdValues.push_back(quadraticFunction(md));
    }

    std::vector<int> expectedSegmentIndices = {0, 0, 1, 1, 1, 2, 2, 2, 2, 3};
    std::vector<int> segmentIndices = RigWellPathGeometryTools::findSegmentIndices(mdValues, tvdValues, fullTvdValues);
    EXPECT_EQ(expectedSegmentIndices.size(), segmentIndices.size());
    for (size_t i = 0; i < expectedSegmentIndices.size(); ++i)
    {
        EXPECT_EQ(expectedSegmentIndices[i], segmentIndices[i]);
    }

    std::vector<double> estimatedFullMDValues = RigWellPathGeometryTools::interpolateMdFromTvd(mdValues, tvdValues, fullTvdValues);
    EXPECT_EQ(estimatedFullMDValues.size(), fullMDValues.size());
    for (size_t i = 0; i < estimatedFullMDValues.size(); ++i)
    {
        EXPECT_NEAR(fullMDValues[i], estimatedFullMDValues[i], TOLERANCE);
    }
}

double cubicFunction(double x)
{
    return 0.000012 * std::pow(x, 3) - 0.0175 * std::pow(x, 2) + 7 * x;
}

TEST(RigWellPathGeometryTools, CubicPath)
{
    std::vector<double> mdValues = {100, 300, 700, 1000};
    std::vector<double> tvdValues;
    for (double md : mdValues)
    {
        tvdValues.push_back(cubicFunction(md));
    }
    std::vector<double> fullMDValues = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
    std::vector<double> fullTvdValues;
    for (double md : fullMDValues)
    {
        fullTvdValues.push_back(cubicFunction(md));
    }

    std::vector<int> expectedSegmentIndices = {0, 0, 1, 1, 1, 2, 2, 2, 2, 3};
    std::vector<int> segmentIndices = RigWellPathGeometryTools::findSegmentIndices(mdValues, tvdValues, fullTvdValues);
    EXPECT_EQ(expectedSegmentIndices.size(), segmentIndices.size());
    for (size_t i = 0; i < expectedSegmentIndices.size(); ++i)
    {
        EXPECT_EQ(expectedSegmentIndices[i], segmentIndices[i]);
    }

    std::vector<double> estimatedFullMDValues = RigWellPathGeometryTools::interpolateMdFromTvd(mdValues, tvdValues, fullTvdValues);
    EXPECT_EQ(estimatedFullMDValues.size(), fullMDValues.size());
    for (size_t i = 0; i < estimatedFullMDValues.size(); ++i)
    {
        EXPECT_DOUBLE_EQ(fullMDValues[i], estimatedFullMDValues[i]);
    }
}

TEST(RigWellPathGeometryTools, CubicPathPoorSampling)
{
    std::vector<double> mdValues = {100, 300, 600, 1000};
    std::vector<double> tvdValues;
    for (double md : mdValues)
    {
        tvdValues.push_back(cubicFunction(md));
    }
    std::vector<double> fullMDValues = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
    std::vector<double> fullTvdValues;
    for (double md : fullMDValues)
    {
        fullTvdValues.push_back(cubicFunction(md));
    }

    std::vector<int> expectedSegmentIndices = {0, 0, 1, 1, 1, 2, 2, 2, 2, 3};
    std::vector<int> segmentIndices = RigWellPathGeometryTools::findSegmentIndices(mdValues, tvdValues, fullTvdValues);
    EXPECT_EQ(expectedSegmentIndices.size(), segmentIndices.size());
    for (size_t i = 0; i < expectedSegmentIndices.size(); ++i)
    {
        EXPECT_EQ(expectedSegmentIndices[i], segmentIndices[i]);
    }

    std::vector<double> estimatedFullMDValues = RigWellPathGeometryTools::interpolateMdFromTvd(mdValues, tvdValues, fullTvdValues);
    EXPECT_EQ(estimatedFullMDValues.size(), fullMDValues.size());
    for (size_t i = 0; i < estimatedFullMDValues.size(); ++i)
    {
        EXPECT_DOUBLE_EQ(fullMDValues[i], estimatedFullMDValues[i]);
    }
}

TEST(RigWellPathGeometryTools, CubicPathVeryPoorSampling)
{
    std::vector<double> mdValues = {150, 400, 800, 900};
    std::vector<double> tvdValues;
    for (double md : mdValues)
    {
        tvdValues.push_back(cubicFunction(md));
    }
    std::vector<double> fullMDValues = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
    std::vector<double> fullTvdValues;
    for (double md : fullMDValues)
    {
        fullTvdValues.push_back(cubicFunction(md));
    }

    std::vector<int> expectedSegmentIndices = {0, 0, 1, 1, 1, 1, 2, 2, 2, 3};
    std::vector<int> segmentIndices = RigWellPathGeometryTools::findSegmentIndices(mdValues, tvdValues, fullTvdValues);
    EXPECT_EQ(expectedSegmentIndices.size(), segmentIndices.size());
    for (size_t i = 0; i < expectedSegmentIndices.size(); ++i)
    {
        EXPECT_EQ(expectedSegmentIndices[i], segmentIndices[i]);
    }

    std::vector<double> estimatedFullMDValues = RigWellPathGeometryTools::interpolateMdFromTvd(mdValues, tvdValues, fullTvdValues);
    EXPECT_EQ(estimatedFullMDValues.size(), fullMDValues.size());
    for (size_t i = 0; i < estimatedFullMDValues.size(); ++i)
    {
        EXPECT_DOUBLE_EQ(fullMDValues[i], estimatedFullMDValues[i]);
    }
}