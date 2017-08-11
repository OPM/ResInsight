/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#pragma once

#include <cstddef>
#include <vector>

class RimEclipseResultCase;
class QString;

//==================================================================================================
/// 
//==================================================================================================

class RigTofAccumulatedPhaseFractionsCalculator
{

public:
    explicit RigTofAccumulatedPhaseFractionsCalculator(RimEclipseResultCase* caseToApply,
                                                       QString wellname, 
                                                       size_t timestep);

    const std::vector<double>&  sortedUniqueTOFValues() const { return m_tofInIncreasingOrder; }
    const std::vector<double>&  accumulatedPhaseFractionsSwat() const { return m_accumulatedPhaseFractionSwat; }
    const std::vector<double>&  accumulatedPhaseFractionsSoil() const { return m_accumulatedPhaseFractionSoil; }
    const std::vector<double>&  accumulatedPhaseFractionsSgas() const { return m_accumulatedPhaseFractionSgas; }
    

private:
    friend class RigTofAccumulatedPhaseFractionsCalculatorTester;
    static void sortTofAndCalculateAccPhaseFraction(const std::vector<double>* tofData, 
                                             const std::vector<double>* fractionData, 
                                             const std::vector<double>* porvResults, 
                                             const std::vector<double>* swatResults, 
                                             const std::vector<double>* soilResults, 
                                             const std::vector<double>* sgasResults,
                                             std::vector<double>& tofInIncreasingOrder,
                                             std::vector<double>& accumulatedPhaseFractionSwat,
                                             std::vector<double>& accumulatedPhaseFractionSoil,
                                             std::vector<double>& accumulatedPhaseFractionSgas);

private:
    std::vector<double> m_tofInIncreasingOrder;
    std::vector<double> m_accumulatedPhaseFractionSwat;
    std::vector<double> m_accumulatedPhaseFractionSgas;
    std::vector<double> m_accumulatedPhaseFractionSoil;

};

class RigTofAccumulatedPhaseFractionsCalculatorTester
{
public:
    static void testSortTofAndCalculateAccPhaseFraction(const std::vector<double>* tofData,
                                                        const std::vector<double>* fractionData,
                                                        const std::vector<double>* porvResults,
                                                        const std::vector<double>* swatResults,
                                                        const std::vector<double>* soilResults,
                                                        const std::vector<double>* sgasResults,
                                                        std::vector<double>& tofInIncreasingOrder,
                                                        std::vector<double>& accumulatedPhaseFractionSwat,
                                                        std::vector<double>& accumulatedPhaseFractionSoil,
                                                        std::vector<double>& accumulatedPhaseFractionSgas)
    {
        RigTofAccumulatedPhaseFractionsCalculator::sortTofAndCalculateAccPhaseFraction(tofData, 
                                                                                       fractionData, 
                                                                                       porvResults, 
                                                                                       swatResults, 
                                                                                       soilResults,
                                                                                       sgasResults,
                                                                                       tofInIncreasingOrder, 
                                                                                       accumulatedPhaseFractionSwat, 
                                                                                       accumulatedPhaseFractionSoil, 
                                                                                       accumulatedPhaseFractionSgas);
    }
};
