/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RiaDefines.h"

#include "RigWellLogCurveData.h"

#include "cvfVector3.h"

#include <vector>

#include <iostream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigWellLogCurveData, createAndAddInterpolatedSegmentValueAndDepths_first )
{
    // Input data
    const std::map<RiaDefines::DepthTypeEnum, std::vector<double>> originalDepths =
        { { RiaDefines::DepthTypeEnum::MEASURED_DEPTH, { 0.0, 20.0, 40.0 } },
          { RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH, { 0.0, 30.0, 60.0 } },
          { RiaDefines::DepthTypeEnum::PSEUDO_LENGTH, { 0.0, 40.0, 80.0 } } };
    const std::vector<double> propertyValues = { 0.0, 100.0, 150.0 };
    const double              eps            = 1e-6;

    // Output data
    const auto                                               resamplingDepthType = RiaDefines::DepthTypeEnum::MEASURED_DEPTH;
    std::vector<double>                                      resampledValues;
    std::map<RiaDefines::DepthTypeEnum, std::vector<double>> resampledDepths;

    // Target data (resampling with MEASURED_DEPTH)
    const double targetDepthValue = 10.0; // Halfway between index 0 and 1 for MEASURED_DEPTH in originalDepths
    const size_t firstIndex       = 0;
    const size_t secondIndex      = firstIndex + 1;

    // Call the function under test
    RigWellLogCurveData::createAndAddInterpolatedSegmentValueAndDepths( resampledValues,
                                                                        resampledDepths,
                                                                        resamplingDepthType,
                                                                        targetDepthValue,
                                                                        firstIndex,
                                                                        secondIndex,
                                                                        originalDepths,
                                                                        propertyValues,
                                                                        eps );

    // Check the results
    ASSERT_EQ( resampledValues.size(), size_t( 1 ) );
    ASSERT_DOUBLE_EQ( resampledValues[0], 50.0 );

    ASSERT_EQ( resampledDepths.size(), size_t( 3 ) );

    ASSERT_EQ( resampledDepths[RiaDefines::DepthTypeEnum::MEASURED_DEPTH].size(), size_t( 1 ) );
    ASSERT_DOUBLE_EQ( resampledDepths[RiaDefines::DepthTypeEnum::MEASURED_DEPTH][0], 10.0 );

    ASSERT_EQ( resampledDepths[RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH].size(), size_t( 1 ) );
    ASSERT_DOUBLE_EQ( resampledDepths[RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH][0], 15.0 );

    ASSERT_EQ( resampledDepths[RiaDefines::DepthTypeEnum::PSEUDO_LENGTH].size(), size_t( 1 ) );
    ASSERT_DOUBLE_EQ( resampledDepths[RiaDefines::DepthTypeEnum::PSEUDO_LENGTH][0], 20.0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigWellLogCurveData, createAndAddInterpolatedSegmentValueAndDepths_second )
{
    // Input data
    const std::map<RiaDefines::DepthTypeEnum, std::vector<double>> originalDepths =
        { { RiaDefines::DepthTypeEnum::MEASURED_DEPTH, { 0.0, 20.0, 40.0 } },
          { RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH, { 0.0, 30.0, 60.0 } },
          { RiaDefines::DepthTypeEnum::PSEUDO_LENGTH, { 0.0, 40.0, 80.0 } } };
    const std::vector<double> propertyValues = { 0.0, 100.0, 150.0 };
    const double              eps            = 1e-6;

    // Output data
    const auto                                               resamplingDepthType = RiaDefines::DepthTypeEnum::MEASURED_DEPTH;
    std::vector<double>                                      resampledValues;
    std::map<RiaDefines::DepthTypeEnum, std::vector<double>> resampledDepths;

    // Target data (resampling with MEASURED_DEPTH)
    const double firstTargetDepthValue  = 10.0; // Halfway between index 0 and 1 for MEASURED_DEPTH in originalDepths
    const double secondTargetDepthValue = 30.0; // Halfway between index 1 and 2 for MEASURED_DEPTH in originalDepths
    const size_t firstIndex             = 0;
    const size_t secondIndex            = 1;
    const size_t thirdIndex             = 2;

    // Call the function under test with interpolating between first and second index
    RigWellLogCurveData::createAndAddInterpolatedSegmentValueAndDepths( resampledValues,
                                                                        resampledDepths,
                                                                        resamplingDepthType,
                                                                        firstTargetDepthValue,
                                                                        firstIndex,
                                                                        firstIndex + 1,
                                                                        originalDepths,
                                                                        propertyValues,
                                                                        eps );

    // Call the function under test with interpolating between second and third index
    RigWellLogCurveData::createAndAddInterpolatedSegmentValueAndDepths( resampledValues,
                                                                        resampledDepths,
                                                                        resamplingDepthType,
                                                                        secondTargetDepthValue,
                                                                        secondIndex,
                                                                        thirdIndex,
                                                                        originalDepths,
                                                                        propertyValues,
                                                                        eps );

    // Check the results
    ASSERT_EQ( resampledValues.size(), size_t( 2 ) );
    ASSERT_DOUBLE_EQ( resampledValues[0], 50.0 );
    ASSERT_DOUBLE_EQ( resampledValues[1], 125.0 );

    ASSERT_EQ( resampledDepths.size(), size_t( 3 ) );

    ASSERT_EQ( resampledDepths[RiaDefines::DepthTypeEnum::MEASURED_DEPTH].size(), size_t( 2 ) );
    ASSERT_DOUBLE_EQ( resampledDepths[RiaDefines::DepthTypeEnum::MEASURED_DEPTH][0], 10.0 );
    ASSERT_DOUBLE_EQ( resampledDepths[RiaDefines::DepthTypeEnum::MEASURED_DEPTH][1], 30.0 );

    ASSERT_EQ( resampledDepths[RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH].size(), size_t( 2 ) );
    ASSERT_DOUBLE_EQ( resampledDepths[RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH][0], 15.0 );
    ASSERT_DOUBLE_EQ( resampledDepths[RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH][1], 45.0 );

    ASSERT_EQ( resampledDepths[RiaDefines::DepthTypeEnum::PSEUDO_LENGTH].size(), size_t( 2 ) );
    ASSERT_DOUBLE_EQ( resampledDepths[RiaDefines::DepthTypeEnum::PSEUDO_LENGTH][0], 20.0 );
    ASSERT_DOUBLE_EQ( resampledDepths[RiaDefines::DepthTypeEnum::PSEUDO_LENGTH][1], 60.0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RigWellLogCurveData, CreateResampledValuesAndDepthsTest )
{
    // Input data
    RiaDefines::DepthTypeEnum                                      resamplingDepthType = RiaDefines::DepthTypeEnum::MEASURED_DEPTH;
    const std::vector<double>                                      targetDepths        = { 0.0, 5.0, 10.0, 15.0 };
    const std::map<RiaDefines::DepthTypeEnum, std::vector<double>> originalDepths =
        { { RiaDefines::DepthTypeEnum::MEASURED_DEPTH, { 0.0, 10.0, 20.0 } },
          { RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH, { 0.0, 20.0, 40.0 } },
          { RiaDefines::DepthTypeEnum::PSEUDO_LENGTH, { 0.0, 30.0, 60.0 } } };
    const std::vector<double> propertyValues = { 0.0, 100.0, 200.0 };

    // Call the function under test
    auto result = RigWellLogCurveData::createResampledValuesAndDepths( resamplingDepthType, targetDepths, originalDepths, propertyValues );

    // Check the results
    std::vector<double>&                                      resampledPropertyValues = result.first;
    std::map<RiaDefines::DepthTypeEnum, std::vector<double>>& resampledDepths         = result.second;
    const auto                                                expectedSize            = targetDepths.size();

    ASSERT_EQ( resampledDepths.size(), originalDepths.size() );
    ASSERT_EQ( resampledDepths[RiaDefines::DepthTypeEnum::MEASURED_DEPTH].size(), expectedSize );
    ASSERT_EQ( resampledDepths[RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH].size(), expectedSize );
    ASSERT_EQ( resampledDepths[RiaDefines::DepthTypeEnum::PSEUDO_LENGTH].size(), expectedSize );

    ASSERT_EQ( resampledPropertyValues.size(), expectedSize );

    // Example assertions for the specific values
    ASSERT_DOUBLE_EQ( resampledPropertyValues[0], 0.0 );
    ASSERT_DOUBLE_EQ( resampledPropertyValues[1], 50.0 );
    ASSERT_DOUBLE_EQ( resampledPropertyValues[2], 100.0 );
    ASSERT_DOUBLE_EQ( resampledPropertyValues[3], 150.0 );

    ASSERT_DOUBLE_EQ( resampledDepths[RiaDefines::DepthTypeEnum::MEASURED_DEPTH][0], 0.0 );
    ASSERT_DOUBLE_EQ( resampledDepths[RiaDefines::DepthTypeEnum::MEASURED_DEPTH][1], 5.0 );
    ASSERT_DOUBLE_EQ( resampledDepths[RiaDefines::DepthTypeEnum::MEASURED_DEPTH][2], 10.0 );
    ASSERT_DOUBLE_EQ( resampledDepths[RiaDefines::DepthTypeEnum::MEASURED_DEPTH][3], 15.0 );

    ASSERT_DOUBLE_EQ( resampledDepths[RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH][0], 0.0 );
    ASSERT_DOUBLE_EQ( resampledDepths[RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH][1], 10.0 );
    ASSERT_DOUBLE_EQ( resampledDepths[RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH][2], 20.0 );
    ASSERT_DOUBLE_EQ( resampledDepths[RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH][3], 30.0 );

    ASSERT_DOUBLE_EQ( resampledDepths[RiaDefines::DepthTypeEnum::PSEUDO_LENGTH][0], 0.0 );
    ASSERT_DOUBLE_EQ( resampledDepths[RiaDefines::DepthTypeEnum::PSEUDO_LENGTH][1], 15.0 );
    ASSERT_DOUBLE_EQ( resampledDepths[RiaDefines::DepthTypeEnum::PSEUDO_LENGTH][2], 30.0 );
    ASSERT_DOUBLE_EQ( resampledDepths[RiaDefines::DepthTypeEnum::PSEUDO_LENGTH][3], 45.0 );
}
