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

#include "RiaDefines.h"

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
    ~RigWellLogCurveData() override;

    void                                      setValuesAndMD(const std::vector<double>& xValues, 
                                                             const std::vector<double>& measuredDepths,
                                                             RiaDefines::DepthUnitType depthUnit,
                                                             bool isExtractionCurve);

    void                                      setValuesWithTVD(const std::vector<double>& xValues, 
                                                               const std::vector<double>& measuredDepths, 
                                                               const std::vector<double>& tvDepths,
                                                               RiaDefines::DepthUnitType depthUnit,
                                                               bool isExtractionCurve);

    const std::vector<double>&                xValues() const;
    const std::vector<double>&                measuredDepths() const;
    const std::vector<double>&                tvDepths() const;
    bool                                      calculateMDRange(double* minMD, double* maxMD) const;

    RiaDefines::DepthUnitType                 depthUnit() const;

    std::vector<double>                       xPlotValues() const;
    std::vector<double>                       trueDepthPlotValues(RiaDefines::DepthUnitType destinationDepthUnit) const;
    std::vector<double>                       measuredDepthPlotValues(RiaDefines::DepthUnitType destinationDepthUnit) const;
    std::vector< std::pair<size_t, size_t> >  polylineStartStopIndices() const;

    cvf::ref<RigWellLogCurveData>             calculateResampledCurveData(double newMeasuredDepthStepSize) const;

private:
    void                                      calculateIntervalsOfContinousValidValues();

    static void                               splitIntervalAtEmptySpace(const std::vector<double>& depthValues, 
                                                                        size_t startIdx, size_t stopIdx, 
                                                                        std::vector< std::pair<size_t, size_t> >* intervals);

    std::vector<double>                       convertDepthValues(RiaDefines::DepthUnitType destinationDepthUnit, const std::vector<double>& originalValues) const;

    static std::vector<double>                convertFromMeterToFeet(const std::vector<double>& valuesInMeter);
    static std::vector<double>                convertFromFeetToMeter(const std::vector<double>& valuesInFeet);

private:
    std::vector<double>                       m_xValues;
    std::vector<double>                       m_measuredDepths;
    std::vector<double>                       m_tvDepths;
    bool                                      m_isExtractionCurve;

    std::vector< std::pair<size_t, size_t> >  m_intervalsOfContinousValidValues;
    
    RiaDefines::DepthUnitType                 m_depthUnit;
};

