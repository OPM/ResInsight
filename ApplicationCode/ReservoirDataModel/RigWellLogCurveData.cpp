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

#include "RigWellLogCurveData.h"

#include "RigCurveDataTools.h"

#include "RiaEclipseUnitTools.h"

#include "cvfMath.h"
#include "cvfAssert.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigWellLogCurveData::RigWellLogCurveData()
{
    m_isExtractionCurve = false;
    m_depthUnit = RiaDefines::UNIT_METER;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigWellLogCurveData::~RigWellLogCurveData()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigWellLogCurveData::setValuesAndMD(const std::vector<double>& xValues, 
                                         const std::vector<double>& measuredDepths,
                                         RiaDefines::DepthUnitType depthUnit,
                                         bool isExtractionCurve)
{
    CVF_ASSERT(xValues.size() == measuredDepths.size());

    m_xValues = xValues;
    m_measuredDepths = measuredDepths;
    m_tvDepths.clear();
    m_depthUnit = depthUnit;

    // Disable depth value filtering is intended to be used for 
    // extraction curve data
    m_isExtractionCurve = isExtractionCurve;

    calculateIntervalsOfContinousValidValues();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigWellLogCurveData::setValuesWithTVD(const std::vector<double>& xValues, 
                                           const std::vector<double>& measuredDepths, 
                                           const std::vector<double>& tvDepths,
                                           RiaDefines::DepthUnitType depthUnit)
{
    CVF_ASSERT(xValues.size() == measuredDepths.size());

    m_xValues = xValues;
    m_measuredDepths = measuredDepths;
    m_tvDepths = tvDepths;
    m_depthUnit = depthUnit;

    // Always use value filtering when TVD is present
    m_isExtractionCurve = true;

    calculateIntervalsOfContinousValidValues(); 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigWellLogCurveData::xValues() const
{
    return m_xValues;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigWellLogCurveData::measuredDepths() const
{
    return m_measuredDepths;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigWellLogCurveData::tvDepths() const
{
    return m_tvDepths;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellLogCurveData::xPlotValues() const
{
    std::vector<double> filteredValues;
    RigCurveDataTools::getValuesByIntervals(m_xValues, m_intervalsOfContinousValidValues, &filteredValues);

    return filteredValues;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellLogCurveData::trueDepthPlotValues(RiaDefines::DepthUnitType destinationDepthUnit) const
{
    std::vector<double> filteredValues;
    if(m_tvDepths.size())
    {
        if(destinationDepthUnit == m_depthUnit)
        {
            RigCurveDataTools::getValuesByIntervals(m_tvDepths, m_intervalsOfContinousValidValues, &filteredValues);
        }
        else
        {
            std::vector<double> convertedValues = convertDepthValues(destinationDepthUnit, m_tvDepths);
            RigCurveDataTools::getValuesByIntervals(convertedValues, m_intervalsOfContinousValidValues, &filteredValues);
        }
    }

    return filteredValues;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellLogCurveData::measuredDepthPlotValues(RiaDefines::DepthUnitType destinationDepthUnit) const
{
    std::vector<double> filteredValues;

    if(destinationDepthUnit == m_depthUnit)
    {
        RigCurveDataTools::getValuesByIntervals(m_measuredDepths, m_intervalsOfContinousValidValues, &filteredValues);
    }
    else
    {
        std::vector<double> convertedValues = convertDepthValues(destinationDepthUnit, m_measuredDepths);
        RigCurveDataTools::getValuesByIntervals(convertedValues, m_intervalsOfContinousValidValues, &filteredValues);
    }

    return filteredValues;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector< std::pair<size_t, size_t> > RigWellLogCurveData::polylineStartStopIndices() const
{
    std::vector< std::pair<size_t, size_t> > lineStartStopIndices;
    RigCurveDataTools::computePolyLineStartStopIndices(m_intervalsOfContinousValidValues, &lineStartStopIndices);

    return lineStartStopIndices;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigWellLogCurveData> RigWellLogCurveData::calculateResampledCurveData(double newMeasuredDepthStepSize) const
{
    std::vector<double> xValues;
    std::vector<double> measuredDepths;

    bool                isTvDepthsAvailable = false;
    std::vector<double> tvDepths;

    if (m_tvDepths.size() > 0) isTvDepthsAvailable = true;

    if(m_measuredDepths.size() > 0)
    {
        double currentMd = m_measuredDepths[0];

        size_t segmentStartIdx = 0;
        while(segmentStartIdx < m_measuredDepths.size() - 1)
        {
            double segmentStartMd = m_measuredDepths[segmentStartIdx];
            double segmentEndMd = m_measuredDepths[segmentStartIdx + 1];
            double segmentStartX = m_xValues[segmentStartIdx];
            double segmentEndX = m_xValues[segmentStartIdx +1];

            double segmentStartTvd = 0.0;
            double segmentEndTvd = 0.0;
            if (isTvDepthsAvailable)
            {
                segmentStartTvd = m_tvDepths[segmentStartIdx];
                segmentEndTvd = m_tvDepths[segmentStartIdx +1];
            }

            while(currentMd <= segmentEndMd)
            {
                measuredDepths.push_back(currentMd);
                double endWeight = (currentMd - segmentStartMd)/ (segmentEndMd - segmentStartMd);
                xValues.push_back((1.0-endWeight) * segmentStartX + endWeight*segmentEndX);

                // The tvd calculation is a simplification. We should use the wellpath, as it might have a better resolution, and have a none-linear shape
                // This is much simpler, and possibly accurate enough ?

                if (isTvDepthsAvailable)
                {
                    tvDepths.push_back((1.0-endWeight) * segmentStartTvd + endWeight*segmentEndTvd);
                }

                currentMd += newMeasuredDepthStepSize;
            }

            segmentStartIdx++;
        }
    }

    cvf::ref<RigWellLogCurveData> reSampledData = new RigWellLogCurveData;

    if (isTvDepthsAvailable)
    {
        reSampledData->setValuesWithTVD(xValues, measuredDepths, tvDepths, m_depthUnit);
    }
    else
    {
        reSampledData->setValuesAndMD(xValues, measuredDepths, m_depthUnit, m_isExtractionCurve);
    }

    return reSampledData;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigWellLogCurveData::calculateIntervalsOfContinousValidValues()
{
    std::vector< std::pair<size_t, size_t> > intervalsOfValidValues;
    RigCurveDataTools::calculateIntervalsOfValidValues(m_xValues, &intervalsOfValidValues, false);

    m_intervalsOfContinousValidValues.clear();

    if (!m_isExtractionCurve)
    {
        m_intervalsOfContinousValidValues = intervalsOfValidValues;
    }
    else
    {
        size_t intervalsCount = intervalsOfValidValues.size();
        for (size_t intIdx = 0; intIdx < intervalsCount; intIdx++)
        {
            std::vector< std::pair<size_t, size_t> > depthValuesIntervals;
            splitIntervalAtEmptySpace(m_measuredDepths, 
                                      intervalsOfValidValues[intIdx].first, intervalsOfValidValues[intIdx].second, 
                                      &depthValuesIntervals);

            for (size_t dvintIdx = 0; dvintIdx < depthValuesIntervals.size(); dvintIdx++)
            {
                m_intervalsOfContinousValidValues.push_back(depthValuesIntervals[dvintIdx]);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Splits the start stop interval between cells that are not close enough. 
/// The depth values are expected to contain pair of depths: Depth at cell enter, and cell leave  
//--------------------------------------------------------------------------------------------------
void RigWellLogCurveData::splitIntervalAtEmptySpace(const std::vector<double>& depthValues, 
                                                               size_t startIdx, size_t stopIdx, 
                                                               std::vector< std::pair<size_t, size_t> >* intervals)
{
    CVF_ASSERT(intervals);

    CVF_ASSERT(startIdx < stopIdx);

    if (stopIdx - startIdx == 1)
    {
        intervals->push_back(std::make_pair(startIdx, stopIdx));
        return;
    }

    // !! TODO: Find a reasonable tolerance
    const double depthDiffTolerance = 0.1;

    // Find intervals containing depth values that should be connected
    size_t intStartIdx = startIdx;
    for (size_t vIdx = startIdx + 1; vIdx < stopIdx; vIdx += 2)
    {
        if (cvf::Math::abs(depthValues[vIdx + 1] - depthValues[vIdx]) > depthDiffTolerance)
        {
            intervals->push_back(std::make_pair(intStartIdx, vIdx));
            intStartIdx = vIdx + 1;
        }
    }

    if (intStartIdx <= stopIdx)
    {
        intervals->push_back(std::make_pair(intStartIdx, stopIdx));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigWellLogCurveData::calculateMDRange(double* minimumDepth, double* maximumDepth) const
{
    CVF_ASSERT(minimumDepth && maximumDepth);

    double minValue = HUGE_VAL;
    double maxValue = -HUGE_VAL;

    for (size_t vIdx = 0; vIdx < m_measuredDepths.size(); vIdx++)
    {
        double value = m_measuredDepths[vIdx];

        if (value < minValue)
        {
            minValue = value;
        }

        if (value > maxValue)
        {
            maxValue = value;
        }
    }

    if (maxValue >= minValue)
    {
        *minimumDepth = minValue;
        *maximumDepth = maxValue;

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaDefines::DepthUnitType RigWellLogCurveData::depthUnit() const
{
    return m_depthUnit;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellLogCurveData::convertFromMeterToFeet(const std::vector<double>& valuesInMeter)
{
    std::vector<double> valuesInFeet(valuesInMeter.size());

    for (size_t i = 0; i < valuesInMeter.size(); i++)
    {
        valuesInFeet[i] = valuesInMeter[i] * RiaEclipseUnitTools::feetPerMeter();
    }

    return valuesInFeet;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellLogCurveData::convertFromFeetToMeter(const std::vector<double>& valuesInFeet)
{
    std::vector<double> valuesInMeter(valuesInFeet.size());

    for (size_t i = 0; i < valuesInFeet.size(); i++)
    {
        valuesInMeter[i] = valuesInFeet[i] / RiaEclipseUnitTools::feetPerMeter();
    }

    return valuesInMeter;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellLogCurveData::convertDepthValues(RiaDefines::DepthUnitType destinationDepthUnit, const std::vector<double>& values) const
{
    CVF_ASSERT(destinationDepthUnit != m_depthUnit);

    if (destinationDepthUnit == RiaDefines::UNIT_METER)
    {
        return convertFromFeetToMeter(values);
    }
    else
    {
        return convertFromMeterToFeet(values);
    }
}
