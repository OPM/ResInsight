/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RigStatisticsCalculator.h"

#include "cvfBase.h"
#include "cvfObject.h"

#include <vector>


//==================================================================================================
/// 
//==================================================================================================
class RigStatisticsDataCache : public cvf::Object
{
public:
    RigStatisticsDataCache(RigStatisticsCalculator* statisticsCalculator);

    void                                    clearAllStatistics();

    void                                    minMaxCellScalarValues(double& min, double& max);
    void                                    minMaxCellScalarValues(size_t timeStepIndex, double& min, double& max);
    void                                    posNegClosestToZero(double& pos, double& neg);
    void                                    posNegClosestToZero(size_t timeStepIndex, double& pos, double& neg);

    void                                    p10p90CellScalarValues(double& p10, double& p90);
    void                                    meanCellScalarValues(double& meanValue);
    const std::vector<size_t>&              cellScalarValuesHistogram();

private:
    void                                    computeStatisticsIfNeeded();

private:
    double                                  m_minValue;
    double                                  m_maxValue;
    bool                                    m_isMaxMinCalculated;

    double                                  m_posClosestToZero;
    double                                  m_negClosestToZero;
    bool                                    m_isClosestToZeroCalculated;

    double                                  m_p10;
    double                                  m_p90;
    double                                  m_meanValue;
    bool                                    m_isMeanCalculated;

    std::vector<size_t>                     m_histogram;

    std::vector<std::pair<double, double> > m_maxMinValuesPrTs;            ///< Max min values for each time step
    std::vector<bool>                       m_isMaxMinPrTsCalculated;
    std::vector<std::pair<double, double> > m_posNegClosestToZeroPrTs;    ///< PosNeg values for each time step
    std::vector<bool>                       m_isClosestToZeroPrTsCalculated;

    cvf::ref<RigStatisticsCalculator>        m_statisticsCalculator;
};

