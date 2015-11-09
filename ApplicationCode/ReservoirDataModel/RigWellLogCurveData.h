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

    void                                      setValuesAndMD(const std::vector<double>& xValues, 
                                                             const std::vector<double>& measuredDepths,
                                                             bool isExtractionCurve);
    void                                      setValuesWithTVD(const std::vector<double>& xValues, 
                                                               const std::vector<double>& measuredDepths, 
                                                               const std::vector<double>& tvDepths );

    const std::vector<double>&                xValues() const;
    const std::vector<double>&                measuredDepths() const;
    bool                                      calculateMDRange(double* minMD, double* maxMD) const;

    std::vector<double>                       xPlotValues() const;
    std::vector<double>                       depthPlotValues() const;
    std::vector< std::pair<size_t, size_t> >  polylineStartStopIndices() const;

private:
    void                                      calculateIntervalsOfContinousValidValues();

    static void                               calculateIntervalsOfValidValues(const std::vector<double>& values, 
                                                                              std::vector< std::pair<size_t, size_t> >* intervals);
    static void                               splitIntervalAtEmptySpace(const std::vector<double>& depthValues, 
                                                                        size_t startIdx, size_t stopIdx, 
                                                                        std::vector< std::pair<size_t, size_t> >* intervals);

    static void                               getValuesByIntervals(const std::vector<double>& values, 
                                                                   const std::vector< std::pair<size_t, size_t> >& intervals, 
                                                                   std::vector<double>* filteredValues);
    static void                               computePolyLineStartStopIndices(const std::vector< std::pair<size_t, size_t> >& intervals, 
                                                                              std::vector< std::pair<size_t, size_t> >* filteredIntervals);

private:
    std::vector<double>                       m_xValues;
    std::vector<double>                       m_measuredDepths;
    std::vector<double>                       m_tvDepths;
    bool                                      m_isExtractionCurve;

    std::vector< std::pair<size_t, size_t> >  m_intervalsOfContinousValidValues;

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
