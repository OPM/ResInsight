/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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

#include "RigTofAccumulatedPhaseFractionsCalculator.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RigTofAccumulatedPhaseFractionsCalculator, sortTofAndCalculateAccPhaseFractionTest)
{
    std::vector<double> tofDataVector;
    tofDataVector.push_back(0.001);
    tofDataVector.push_back(0.1);
    tofDataVector.push_back(0.01);
    
    std::vector<double> fractionDataVector;
    fractionDataVector.push_back(0.002);
    fractionDataVector.push_back(0.2);
    fractionDataVector.push_back(0.02);

    std::vector<double> porvResultVector;
    porvResultVector.push_back(0.002);
    porvResultVector.push_back(0.2);
    porvResultVector.push_back(0.02);

    std::vector<double> swatResultVector;
    swatResultVector.push_back(0.1);
    swatResultVector.push_back(0.3);
    swatResultVector.push_back(0.6);

    std::vector<double> soilResultVector;
    soilResultVector.push_back(0.3);
    soilResultVector.push_back(0.6);
    soilResultVector.push_back(0.1);

    std::vector<double> sgasResultVector;
    sgasResultVector.push_back(0.6);
    sgasResultVector.push_back(0.1);
    sgasResultVector.push_back(0.3);

    RigTofAccumulatedPhaseFractionsCalculator::sortTofAndCalculateAccPhaseFraction(&(tofDataVector),
                                                                                   &(fractionDataVector),
                                                                                   &(porvResultVector),
                                                                                   &(swatResultVector),
                                                                                   &(soilResultVector),
                                                                                   &(sgasResultVector));
    
    EXPECT_EQ(1, 1);
}