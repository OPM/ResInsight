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

#pragma once

#include <QDateTime>
#include <QString>

#include <map>
#include <vector>

class RigAccWellFlowCalculator;

class RimWellAllocationOverTimeCollection
{
public:
    RimWellAllocationOverTimeCollection( const std::vector<QDateTime>&                        timeStepDates,
                                         const std::map<QDateTime, RigAccWellFlowCalculator>& timeStepAndCalculatorPairs );

    const std::vector<QDateTime>                         timeStepDates() const { return m_timeStepDates; }
    const std::map<QString, std::map<QDateTime, double>> wellValuesMap() const { return m_wellValuesMap; }

    void fillWithFlowRateFractionValues();
    void fillWithFlowRatePercentageValues();
    void fillWithFlowRateValues();
    void fillWithFlowVolumeValues();
    void fillWithAccumulatedFlowVolumeValues( double smallContributionsThreshold );
    void fillWithAccumulatedFlowVolumeFractionValues( double smallContributionsThreshold );
    void fillWithAccumulatedFlowVolumePercentageValues( double smallContributionsThreshold );

private:
    enum class FractionOrPercentage
    {
        FRACTION,
        PERCENTAGE
    };

    void fillWithAccumulatedFlowVolumeFractionOrPercentageValues( FractionOrPercentage selection,
                                                                  double               smallContributionsThreshold );

    void groupAccumulatedFlowVolumes( std::map<QString, std::map<QDateTime, double>>& rWellValuesMap, double threshold );
    void groupAccumulatedFlowVolumeFractionsOrPercentages( std::map<QString, std::map<QDateTime, double>>& rWellValuesMap,
                                                           double                                          threshold );

private:
    std::map<QDateTime, RigAccWellFlowCalculator>  m_timeStepAndCalculatorPairs;
    std::vector<QDateTime>                         m_timeStepDates;
    std::map<QString, std::map<QDateTime, double>> m_defaultWellValuesMap;
    std::map<QString, std::map<QDateTime, double>> m_wellValuesMap;
};
