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

#include "RiaCurveDataTools.h"
#include "RiaEclipseUnitTools.h"
#include "RiaWellLogUnitTools.h"

#include "cvfAssert.h"
#include "cvfMath.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellLogCurveData::RigWellLogCurveData()
{
    m_isExtractionCurve = false;
    m_depthUnit         = RiaDefines::UNIT_METER;
    m_xUnitString       = RiaWellLogUnitTools<double>::noUnitString();
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
void RigWellLogCurveData::setValuesAndDepths( const std::vector<double>& xValues,
                                              const std::vector<double>& depths,
                                              RiaDefines::DepthTypeEnum  depthType,
                                              double                     rkbDiff,
                                              RiaDefines::DepthUnitType  depthUnit,
                                              bool                       isExtractionCurve )
{
    CVF_ASSERT( xValues.size() == depths.size() );

    m_xValues           = xValues;
    m_depths[depthType] = depths;
    m_depthUnit         = depthUnit;
    m_rkbDiff           = rkbDiff;

    // Disable depth value filtering is intended to be used for
    // extraction curve data
    m_isExtractionCurve = isExtractionCurve;

    calculateIntervalsOfContinousValidValues();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellLogCurveData::setValuesAndDepths( const std::vector<double>&                                      xValues,
                                              const std::map<RiaDefines::DepthTypeEnum, std::vector<double>>& depths,
                                              double                                                          rkbDiff,
                                              RiaDefines::DepthUnitType                                       depthUnit,
                                              bool isExtractionCurve )
{
    for ( auto it = depths.begin(); it != depths.end(); ++it )
    {
        CVF_ASSERT( xValues.size() == it->second.size() );
    }

    m_xValues   = xValues;
    m_depths    = depths;
    m_depthUnit = depthUnit;
    m_rkbDiff   = rkbDiff;

    // Disable depth value filtering is intended to be used for
    // extraction curve data
    m_isExtractionCurve = isExtractionCurve;

    calculateIntervalsOfContinousValidValues();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellLogCurveData::setXUnits( const QString& xUnitString )
{
    m_xUnitString = xUnitString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellLogCurveData::xValues() const
{
    return m_xValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellLogCurveData::xValues( const QString& units ) const
{
    std::vector<double> convertedValues;
    if ( units != m_xUnitString && RiaWellLogUnitTools<double>::convertValues( depths( RiaDefines::TRUE_VERTICAL_DEPTH_RKB ),
                                                                               m_xValues,
                                                                               &convertedValues,
                                                                               m_xUnitString,
                                                                               units ) )
    {
        return convertedValues;
    }
    return m_xValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigWellLogCurveData::xUnits() const
{
    return m_xUnitString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellLogCurveData::depths( RiaDefines::DepthTypeEnum depthType ) const
{
    auto it = m_depths.find( depthType );
    if ( it != m_depths.end() )
    {
        return it->second;
    }

    if ( m_rkbDiff != 0.0 )
    {
        if ( depthType == RiaDefines::TRUE_VERTICAL_DEPTH_RKB && m_depths.count( RiaDefines::TRUE_VERTICAL_DEPTH ) )
        {
            std::vector<double> tvds = depths( RiaDefines::TRUE_VERTICAL_DEPTH );
            for ( double& tvdValue : tvds )
            {
                tvdValue += m_rkbDiff;
            }
            return tvds;
        }
        else if ( depthType == RiaDefines::TRUE_VERTICAL_DEPTH && m_depths.count( RiaDefines::TRUE_VERTICAL_DEPTH_RKB ) )
        {
            std::vector<double> tvds = depths( RiaDefines::TRUE_VERTICAL_DEPTH_RKB );
            for ( double& tvdValue : tvds )
            {
                tvdValue -= m_rkbDiff;
            }
            return tvds;
        }
    }
    return std::vector<double>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RiaDefines::DepthTypeEnum> RigWellLogCurveData::availableDepthTypes() const
{
    std::set<RiaDefines::DepthTypeEnum> depthTypes;

    for ( auto depthValuePair : m_depths )
    {
        depthTypes.insert( depthValuePair.first );
    }

    if ( m_rkbDiff != 0.0 )
    {
        if ( depthTypes.count( RiaDefines::TRUE_VERTICAL_DEPTH ) && !depthTypes.count( RiaDefines::TRUE_VERTICAL_DEPTH_RKB ) )
        {
            depthTypes.insert( RiaDefines::TRUE_VERTICAL_DEPTH_RKB );
        }
        else if ( depthTypes.count( RiaDefines::TRUE_VERTICAL_DEPTH_RKB ) &&
                  !depthTypes.count( RiaDefines::TRUE_VERTICAL_DEPTH ) )
        {
            depthTypes.insert( RiaDefines::TRUE_VERTICAL_DEPTH );
        }
    }

    return depthTypes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellLogCurveData::xPlotValues() const
{
    std::vector<double> filteredValues;
    RiaCurveDataTools::getValuesByIntervals( m_xValues, m_intervalsOfContinousValidValues, &filteredValues );

    return filteredValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellLogCurveData::depthPlotValues( RiaDefines::DepthTypeEnum depthType,
                                                          RiaDefines::DepthUnitType destinationDepthUnit ) const
{
    std::vector<double> filteredValues;

    const std::vector<double> depthValues = depths( depthType );
    if ( !depthValues.empty() )
    {
        if ( destinationDepthUnit == m_depthUnit )
        {
            RiaCurveDataTools::getValuesByIntervals( depthValues, m_intervalsOfContinousValidValues, &filteredValues );
        }
        else
        {
            std::vector<double> convertedValues =
                RiaWellLogUnitTools<double>::convertDepths( depthValues, m_depthUnit, destinationDepthUnit );
            RiaCurveDataTools::getValuesByIntervals( convertedValues, m_intervalsOfContinousValidValues, &filteredValues );
        }
    }

    return filteredValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<size_t, size_t>> RigWellLogCurveData::polylineStartStopIndices() const
{
    return RiaCurveDataTools::computePolyLineStartStopIndices( m_intervalsOfContinousValidValues );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigWellLogCurveData> RigWellLogCurveData::calculateResampledCurveData( double newMeasuredDepthStepSize ) const
{
    std::vector<double> xValues;
    std::vector<double> measuredDepths;

    bool                isTVDAvailable = false;
    std::vector<double> tvDepths;

    auto mdIt  = m_depths.find( RiaDefines::MEASURED_DEPTH );
    auto tvdIt = m_depths.find( RiaDefines::TRUE_VERTICAL_DEPTH );

    if ( tvdIt != m_depths.end() && !tvdIt->second.empty() ) isTVDAvailable = true;

    if ( mdIt != m_depths.end() && !mdIt->second.empty() )
    {
        double currentMd = mdIt->second.front();

        size_t segmentStartIdx = 0;
        while ( segmentStartIdx < mdIt->second.size() - 1 )
        {
            double segmentStartMd = mdIt->second[segmentStartIdx];
            double segmentEndMd   = mdIt->second[segmentStartIdx + 1];
            double segmentStartX  = m_xValues[segmentStartIdx];
            double segmentEndX    = m_xValues[segmentStartIdx + 1];

            double segmentStartTvd = 0.0;
            double segmentEndTvd   = 0.0;
            if ( isTVDAvailable )
            {
                segmentStartTvd = tvdIt->second[segmentStartIdx];
                segmentEndTvd   = tvdIt->second[segmentStartIdx + 1];
            }

            while ( currentMd <= segmentEndMd )
            {
                measuredDepths.push_back( currentMd );
                double endWeight = ( currentMd - segmentStartMd ) / ( segmentEndMd - segmentStartMd );
                xValues.push_back( ( 1.0 - endWeight ) * segmentStartX + endWeight * segmentEndX );

                // The tvd calculation is a simplification. We should use the wellpath, as it might have a better
                // resolution, and have a none-linear shape This is much simpler, and possibly accurate enough ?

                if ( isTVDAvailable )
                {
                    tvDepths.push_back( ( 1.0 - endWeight ) * segmentStartTvd + endWeight * segmentEndTvd );
                }

                currentMd += newMeasuredDepthStepSize;
            }

            segmentStartIdx++;
        }
    }

    cvf::ref<RigWellLogCurveData> reSampledData = new RigWellLogCurveData;

    if ( isTVDAvailable )
    {
        std::map<RiaDefines::DepthTypeEnum, std::vector<double>> resampledDepths =
            {{RiaDefines::TRUE_VERTICAL_DEPTH, tvDepths}, {RiaDefines::MEASURED_DEPTH, measuredDepths}};
        reSampledData->setValuesAndDepths( xValues, resampledDepths, m_rkbDiff, m_depthUnit, true );
    }
    else
    {
        reSampledData->setValuesAndDepths( xValues, measuredDepths, RiaDefines::MEASURED_DEPTH, 0.0, m_depthUnit, m_isExtractionCurve );
    }

    return reSampledData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellLogCurveData::calculateIntervalsOfContinousValidValues()
{
    std::vector<std::pair<size_t, size_t>> intervalsOfValidValues =
        RiaCurveDataTools::calculateIntervalsOfValidValues( m_xValues, false );

    m_intervalsOfContinousValidValues.clear();

    if ( !m_isExtractionCurve )
    {
        m_intervalsOfContinousValidValues = intervalsOfValidValues;
    }
    else
    {
        size_t intervalsCount = intervalsOfValidValues.size();
        for ( size_t intIdx = 0; intIdx < intervalsCount; intIdx++ )
        {
            std::vector<std::pair<size_t, size_t>> depthValuesIntervals;
            splitIntervalAtEmptySpace( m_depths[RiaDefines::MEASURED_DEPTH],
                                       intervalsOfValidValues[intIdx].first,
                                       intervalsOfValidValues[intIdx].second,
                                       &depthValuesIntervals );

            for ( size_t dvintIdx = 0; dvintIdx < depthValuesIntervals.size(); dvintIdx++ )
            {
                m_intervalsOfContinousValidValues.push_back( depthValuesIntervals[dvintIdx] );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Splits the start stop interval between cells that are not close enough.
//--------------------------------------------------------------------------------------------------
void RigWellLogCurveData::splitIntervalAtEmptySpace( const std::vector<double>&              depthValues,
                                                     size_t                                  startIdx,
                                                     size_t                                  stopIdx,
                                                     std::vector<std::pair<size_t, size_t>>* intervals )
{
    CVF_ASSERT( intervals );

    CVF_ASSERT( startIdx <= stopIdx );

    if ( stopIdx - startIdx <= 1 )
    {
        intervals->push_back( std::make_pair( startIdx, stopIdx ) );
        return;
    }

    // !! TODO: Find a reasonable tolerance
    const double depthDiffTolerance = 0.1;

    // Find intervals containing depth values that should be connected:
    //
    // vIdx = 0 is the first point of a well, usually outside of the model. Further depth values are
    // organized in pairs of depths (in and out of a cell), and sometimes the depths varies slightly. If
    // the distance between a depth pair is larger than the depthDiffTolerance, the two sections will be split
    // into two intervals.
    //
    // The first pair is located at vIdx = 1 & 2. If startIdx = 0, an offset of 1 is added to vIdx, to access
    // that pair in the loop. If startIdx = 1 (can happen if the start point is inside of the model and invalid),
    // the offset is not needed.

    size_t intervalStartIdx = startIdx;
    size_t offset           = 1 - startIdx % 2;
    for ( size_t vIdx = startIdx + offset; vIdx < stopIdx; vIdx += 2 )
    {
        if ( cvf::Math::abs( depthValues[vIdx + 1] - depthValues[vIdx] ) > depthDiffTolerance )
        {
            intervals->push_back( std::make_pair( intervalStartIdx, vIdx ) );
            intervalStartIdx = vIdx + 1;
        }
    }

    if ( intervalStartIdx <= stopIdx )
    {
        intervals->push_back( std::make_pair( intervalStartIdx, stopIdx ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigWellLogCurveData::calculateDepthRange( RiaDefines::DepthTypeEnum depthType,
                                               RiaDefines::DepthUnitType depthUnit,
                                               double*                   minimumDepth,
                                               double*                   maximumDepth ) const
{
    CVF_ASSERT( minimumDepth && maximumDepth );

    double minValue = HUGE_VAL;
    double maxValue = -HUGE_VAL;

    std::vector<double> depthValues = depthPlotValues( depthType, depthUnit );
    for ( size_t vIdx = 0; vIdx < depthValues.size(); vIdx++ )
    {
        double value = depthValues[vIdx];

        if ( value < minValue )
        {
            minValue = value;
        }

        if ( value > maxValue )
        {
            maxValue = value;
        }
    }

    if ( maxValue >= minValue )
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
