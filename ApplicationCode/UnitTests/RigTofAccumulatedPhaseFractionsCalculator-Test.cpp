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
    porvResultVector.push_back(1);
    porvResultVector.push_back(1);
    porvResultVector.push_back(1.5);

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


    std::vector<double> accumulatedPhaseFractionSwat;
    std::vector<double> accumulatedPhaseFractionSoil;
    std::vector<double> accumulatedPhaseFractionSgas;
    std::vector<double> tofInIncreasingOrder;

    RigTofAccumulatedPhaseFractionsCalculator::sortTofAndCalculateAccPhaseFraction(&(tofDataVector),
                                                                                   &(fractionDataVector),
                                                                                   &(porvResultVector),
                                                                                   &(swatResultVector),
                                                                                   &(soilResultVector),
                                                                                   &(sgasResultVector), 
                                                                                   tofInIncreasingOrder,
                                                                                   accumulatedPhaseFractionSwat,
                                                                                   accumulatedPhaseFractionSoil,
                                                                                   accumulatedPhaseFractionSgas
                                                                                   );
    
    EXPECT_LT(tofInIncreasingOrder[0], tofInIncreasingOrder[1]);
    EXPECT_LT(tofInIncreasingOrder[1], tofInIncreasingOrder[2]);
    
    EXPECT_DOUBLE_EQ(accumulatedPhaseFractionSwat[0], 0.1000);
    EXPECT_DOUBLE_EQ(accumulatedPhaseFractionSoil[1], 0.1125);
    EXPECT_LT(accumulatedPhaseFractionSgas[2] - 0.13017, 0.00001);
}

