//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#include "cvfBase.h"
#include "cvfPerformanceInfo.h"


#include "gtest/gtest.h"

using namespace cvf;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PerformanceInfoTest, BasicConstruction)
{
    PerformanceInfo perf;

    ASSERT_EQ(0, perf.totalDrawTime);
    ASSERT_EQ(0, perf.computeVisiblePartsTime);
    ASSERT_EQ(0, perf.buildRenderQueueTime);
    ASSERT_EQ(0, perf.sortRenderQueueTime);
    ASSERT_EQ(0, perf.renderEngineTime);
    ASSERT_EQ(0u, perf.visiblePartsCount);
    ASSERT_EQ(0u, perf.vertexCount);
    ASSERT_EQ(0u, perf.triangleCount);
    ASSERT_EQ(0u, perf.openGLPrimitiveCount);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PerformanceInfoTest, Clear)
{
    PerformanceInfo perf;

    perf.totalDrawTime = 1;
    perf.computeVisiblePartsTime = 1;
    perf.buildRenderQueueTime = 1;
    perf.sortRenderQueueTime = 1;
    perf.renderEngineTime = 1;
    perf.visiblePartsCount = 1;
    perf.vertexCount = 1;
    perf.triangleCount = 1;
    perf.openGLPrimitiveCount = 1;

    perf.clear();

    ASSERT_EQ(0, perf.totalDrawTime);
    ASSERT_EQ(0, perf.computeVisiblePartsTime);
    ASSERT_EQ(0, perf.buildRenderQueueTime);
    ASSERT_EQ(0, perf.sortRenderQueueTime);
    ASSERT_EQ(0, perf.renderEngineTime);
    ASSERT_EQ(0u, perf.visiblePartsCount);
    ASSERT_EQ(0u, perf.vertexCount);
    ASSERT_EQ(0u, perf.triangleCount);
    ASSERT_EQ(0u, perf.openGLPrimitiveCount);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PerformanceInfoTest, Update)
{
    PerformanceInfo perf;

    perf.totalDrawTime = 1;
    perf.computeVisiblePartsTime = 2;
    perf.buildRenderQueueTime = 3;
    perf.sortRenderQueueTime = 4;
    perf.renderEngineTime = 5;
    perf.visiblePartsCount = 6;
    perf.vertexCount = 7;
    perf.triangleCount = 8;
    perf.openGLPrimitiveCount = 10;

    PerformanceInfo perf2;
    perf2.update(perf);

    ASSERT_EQ(1, perf2.totalDrawTime);
    ASSERT_EQ(2, perf2.computeVisiblePartsTime);
    ASSERT_EQ(3, perf2.buildRenderQueueTime);
    ASSERT_EQ(4, perf2.sortRenderQueueTime);
    ASSERT_EQ(5, perf2.renderEngineTime);
    ASSERT_EQ(6u, perf2.visiblePartsCount);
    ASSERT_EQ(7u, perf2.vertexCount);
    ASSERT_EQ(8u, perf2.triangleCount);
    ASSERT_EQ(10u, perf2.openGLPrimitiveCount);

    perf2.update(perf);

    ASSERT_EQ(2, perf2.totalDrawTime);
    ASSERT_EQ(4, perf2.computeVisiblePartsTime);
    ASSERT_EQ(6, perf2.buildRenderQueueTime);
    ASSERT_EQ(8, perf2.sortRenderQueueTime);
    ASSERT_EQ(10, perf2.renderEngineTime);
    ASSERT_EQ(12u, perf2.visiblePartsCount);
    ASSERT_EQ(14u, perf2.vertexCount);
    ASSERT_EQ(16u, perf2.triangleCount);
    ASSERT_EQ(20u, perf2.openGLPrimitiveCount);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(PerformanceInfoTest, History)
{
    PerformanceInfo summary;
    PerformanceInfo perf;

    perf.totalDrawTime = 1;
    summary.resetCurrentTimers();
    summary.update(perf);
    EXPECT_DOUBLE_EQ(1.0, summary.averageTotalDrawTime());
    
    perf.totalDrawTime = 1;
    summary.resetCurrentTimers();
    summary.update(perf);
    EXPECT_DOUBLE_EQ(1.0, summary.averageTotalDrawTime());
    
    perf.totalDrawTime = 3;
    summary.resetCurrentTimers();
    summary.update(perf);
    EXPECT_DOUBLE_EQ(5.0/3.0, summary.averageTotalDrawTime());

    perf.totalDrawTime = 3;
    summary.resetCurrentTimers();
    summary.update(perf);
    EXPECT_DOUBLE_EQ(8.0/4.0, summary.averageTotalDrawTime());

    perf.totalDrawTime = 1;
    summary.resetCurrentTimers();
    summary.update(perf);
    EXPECT_DOUBLE_EQ(9.0/5.0, summary.averageTotalDrawTime());
}
