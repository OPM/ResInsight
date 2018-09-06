/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template<class T>
class RiaWeightedAverageCalculator
{
public:
    RiaWeightedAverageCalculator();

    void   addValueAndWeight(T value, double weight);

    T      weightedAverage() const;
    double aggregatedWeight() const;
    bool   validAggregatedWeight() const;
private:
    T      m_aggregatedValue;
    double m_aggregatedWeight;
};

#include "RiaWeightedAverageCalculator.inl"