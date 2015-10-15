/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "cvfBase.h"
#include "cvfObject.h"

#include <vector>

class RigWellLogCurveDataTestInterface;

//==================================================================================================
/// 
//==================================================================================================
class RigWellLogCurveData : public cvf::Object
{
public:
    RigWellLogCurveData();
    virtual ~RigWellLogCurveData();

    void setPoints(const std::vector<double>& xValues, const std::vector<double>& yValues);

    const std::vector<double>&                      xValues() const;
    const std::vector<double>&                      yValues() const;
    const std::vector< std::pair<size_t, size_t> >& filteredIntervals() const;

    std::vector<double>                             validXValues() const;
    std::vector<double>                             validYValues() const;
    std::vector< std::pair<size_t, size_t> >        validPointsIntervals() const;

private:
    void        calculateValidXValuesIntervals();

    static void calculateIntervalsOfValidValues(const std::vector<double>& values, std::vector< std::pair<size_t, size_t> >* intervals);
    static void pickValuesFromIntervals(const std::vector<double>& values, const std::vector< std::pair<size_t, size_t> >& intervals, std::vector<double>* filteredValues);
    static void computeFilteredIntervals(const std::vector< std::pair<size_t, size_t> >& intervals, std::vector< std::pair<size_t, size_t> >* filteredIntervals);

private:
    std::vector<double>                         m_xValues;
    std::vector<double>                         m_yValues;
    std::vector< std::pair<size_t, size_t> >    m_validXValuesIntervals;

friend class RigWellLogCurveDataTestInterface;
};

//==================================================================================================
/// 
//==================================================================================================
class RigWellLogCurveDataTestInterface
{
public:
    static void calculateIntervalsOfValidValues(const std::vector<double>& values, std::vector< std::pair<size_t, size_t> >* intervals)
    {
        RigWellLogCurveData::calculateIntervalsOfValidValues(values, intervals);
    }
};
