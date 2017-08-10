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

    double sumForOneTOF = accumulatedPhaseFractionSwat[2]
        + accumulatedPhaseFractionSoil[2]
        + accumulatedPhaseFractionSgas[2];
    EXPECT_DOUBLE_EQ(sumForOneTOF, 1.00);
    
    EXPECT_DOUBLE_EQ(accumulatedPhaseFractionSwat[0], 0.1000);
    EXPECT_DOUBLE_EQ(accumulatedPhaseFractionSoil[1], 0.1125);
    EXPECT_LT(accumulatedPhaseFractionSgas[2] - 0.13017, 0.00001);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RigTofAccumulatedPhaseFractionsCalculator, sortTofAndCalculateAccPhaseFractionTest2)
{
    std::vector<double> tofDataVector2;
    tofDataVector2.push_back(0.001);
    tofDataVector2.push_back(0.1);
    tofDataVector2.push_back(0.01);
    tofDataVector2.push_back(0.1);

    std::vector<double> fractionDataVector2;
    fractionDataVector2.push_back(0.002);
    fractionDataVector2.push_back(0.2);
    fractionDataVector2.push_back(0.02);
    fractionDataVector2.push_back(0.02);

    std::vector<double> porvResultVector2;
    porvResultVector2.push_back(1);
    porvResultVector2.push_back(1);
    porvResultVector2.push_back(1.5);
    porvResultVector2.push_back(1.5);

    std::vector<double> swatResultVector2;
    swatResultVector2.push_back(0.1);
    swatResultVector2.push_back(0.3);
    swatResultVector2.push_back(0.6);
    swatResultVector2.push_back(0.6);

    std::vector<double> soilResultVector2;
    soilResultVector2.push_back(0.3);
    soilResultVector2.push_back(0.6);
    soilResultVector2.push_back(0.1);
    soilResultVector2.push_back(0.1);

    std::vector<double> sgasResultVector2;
    sgasResultVector2.push_back(0.6);
    sgasResultVector2.push_back(0.1);
    sgasResultVector2.push_back(0.3);
    sgasResultVector2.push_back(0.3);

    std::vector<double> accumulatedPhaseFractionSwat2;
    std::vector<double> accumulatedPhaseFractionSoil2;
    std::vector<double> accumulatedPhaseFractionSgas2;
    std::vector<double> tofInIncreasingOrder2;

    RigTofAccumulatedPhaseFractionsCalculator::sortTofAndCalculateAccPhaseFraction(&(tofDataVector2),
                                                                                   &(fractionDataVector2),
                                                                                   &(porvResultVector2),
                                                                                   &(swatResultVector2),
                                                                                   &(soilResultVector2),
                                                                                   &(sgasResultVector2),
                                                                                   tofInIncreasingOrder2,
                                                                                   accumulatedPhaseFractionSwat2,
                                                                                   accumulatedPhaseFractionSoil2,
                                                                                   accumulatedPhaseFractionSgas2);
    EXPECT_EQ(tofInIncreasingOrder2.size(), 3);

    double sumForOneTOF = accumulatedPhaseFractionSwat2[2] 
                        + accumulatedPhaseFractionSoil2[2]
                        + accumulatedPhaseFractionSgas2[2];
    EXPECT_DOUBLE_EQ(sumForOneTOF, 1.00);

    EXPECT_LT(tofInIncreasingOrder2[0], tofInIncreasingOrder2[1]);
    EXPECT_LT(tofInIncreasingOrder2[1], tofInIncreasingOrder2[2]);

    EXPECT_DOUBLE_EQ(accumulatedPhaseFractionSwat2[0], 0.1000);
    EXPECT_DOUBLE_EQ(accumulatedPhaseFractionSoil2[1], 0.1125);
    EXPECT_LT(accumulatedPhaseFractionSgas2[2] - 0.149618, 0.00001);

}

