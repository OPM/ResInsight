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
    m_depthUnit         = RiaDefines::DepthUnitType::UNIT_METER;
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
    if ( units != m_xUnitString &&
         RiaWellLogUnitTools<double>::convertValues( depths( RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH_RKB ),
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

    if ( depthType == RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH_RKB &&
         m_depths.count( RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH ) )
    {
        std::vector<double> tvds = depths( RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH );
        for ( double& tvdValue : tvds )
        {
            tvdValue += m_rkbDiff;
        }
        return tvds;
    }
    else if ( depthType == RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH &&
              m_depths.count( RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH_RKB ) )
    {
        std::vector<double> tvds = depths( RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH_RKB );
        for ( double& tvdValue : tvds )
        {
            tvdValue -= m_rkbDiff;
        }
        return tvds;
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
        if ( depthTypes.count( RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH ) &&
             !depthTypes.count( RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH_RKB ) )
        {
            depthTypes.insert( RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH_RKB );
        }
        else if ( depthTypes.count( RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH_RKB ) &&
                  !depthTypes.count( RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH ) )
        {
            depthTypes.insert( RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH );
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

    auto mdIt  = m_depths.find( RiaDefines::DepthTypeEnum::MEASURED_DEPTH );
    auto tvdIt = m_depths.find( RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH );

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
            { { RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH, tvDepths },
              { RiaDefines::DepthTypeEnum::MEASURED_DEPTH, measuredDepths } };
        reSampledData->setValuesAndDepths( xValues, resampledDepths, m_rkbDiff, m_depthUnit, true );
    }
    else
    {
        reSampledData->setValuesAndDepths( xValues,
                                           measuredDepths,
                                           RiaDefines::DepthTypeEnum::MEASURED_DEPTH,
                                           0.0,
                                           m_depthUnit,
                                           m_isExtractionCurve );
    }

    return reSampledData;
}

void RigWellLogCurveData::interpolateSegment( RiaDefines::DepthTypeEnum resamplingDepthType,
                                              double                    depthValue,
                                              size_t                    firstIndex,
                                              std::vector<double>&      xValues,
                                              std::map<RiaDefines::DepthTypeEnum, std::vector<double>>& resampledDepths,
                                              const double                                              eps ) const
{
    auto depthIt = m_depths.find( resamplingDepthType );

    size_t secondIndex = firstIndex + 1;

    double depth0 = depthIt->second[firstIndex];
    double depth1 = depthIt->second[secondIndex];
    double x0     = m_xValues[firstIndex];
    double x1     = m_xValues[secondIndex];
    double slope  = 0.0;
    if ( std::fabs( depth1 - depth0 ) > eps )
    {
        slope = ( x1 - x0 ) / ( depth1 - depth0 );
    }
    double xValue = slope * ( depthValue - depth0 ) + x0;
    xValues.push_back( xValue );

    for ( auto depthTypeValuesPair : m_depths )
    {
        if ( depthTypeValuesPair.first != resamplingDepthType )
        {
            double otherDepth0 = depthTypeValuesPair.second[0];
            double otherDepth1 = depthTypeValuesPair.second[1];
            double otherSlope  = ( otherDepth1 - otherDepth0 ) / ( depth1 - depth0 );
            resampledDepths[depthTypeValuesPair.first].push_back( otherSlope * ( depthValue - depth0 ) + otherDepth0 );
        }
    }
}

bool isLeftOf( double x1, double x2, bool reverseOrder, double eps )
{
    if ( reverseOrder )
    {
        return x1 - x2 > eps;
    }
    return x2 - x1 > eps;
}

bool isRightOf( double x1, double x2, bool reverseOrder, double eps )
{
    return isLeftOf( x2, x1, reverseOrder, eps );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigWellLogCurveData> RigWellLogCurveData::calculateResampledCurveData( RiaDefines::DepthTypeEnum resamplingDepthType,
                                                                                const std::vector<double>& depths ) const
{
    const double eps = 1.0e-8;

    std::vector<double> xValues;

    std::map<RiaDefines::DepthTypeEnum, std::vector<double>> resampledDepths;
    resampledDepths.insert( std::make_pair( resamplingDepthType, depths ) );

    auto depthIt = m_depths.find( resamplingDepthType );

    cvf::ref<RigWellLogCurveData> reSampledData = new RigWellLogCurveData;

    if ( depthIt == m_depths.end() || depthIt->second.empty() ) return reSampledData;

    bool reverseOrder = resamplingDepthType == RiaDefines::DepthTypeEnum::CONNECTION_NUMBER;

    size_t segmentSearchStartIdx = 0;
    for ( auto depth : depths )
    {
        bool foundPoint = false;
        for ( size_t segmentStartIdx = segmentSearchStartIdx; segmentStartIdx < depthIt->second.size(); ++segmentStartIdx )
        {
            if ( std::fabs( depthIt->second[segmentStartIdx] - depth ) < eps ) // already have this depth point,
                                                                               // reuse it
            {
                xValues.push_back( m_xValues[segmentStartIdx] );
                // Copy all depth types for this segment
                for ( auto depthTypeValuesPair : m_depths )
                {
                    if ( depthTypeValuesPair.first != resamplingDepthType )
                    {
                        resampledDepths[depthTypeValuesPair.first].push_back( depthTypeValuesPair.second[segmentStartIdx] );
                    }
                }
                segmentSearchStartIdx = segmentStartIdx + 1;
                foundPoint            = true;
                break;
            }
            else if ( segmentStartIdx < depthIt->second.size() - 1 )
            {
                double minDepthSegment = std::min( depthIt->second[segmentStartIdx], depthIt->second[segmentStartIdx + 1] );
                double maxDepthSegment = std::max( depthIt->second[segmentStartIdx], depthIt->second[segmentStartIdx + 1] );
                if ( cvf::Math::valueInRange( depth, minDepthSegment, maxDepthSegment ) )
                {
                    interpolateSegment( resamplingDepthType, depth, segmentStartIdx, xValues, resampledDepths, eps );
                    segmentSearchStartIdx = segmentStartIdx;
                    foundPoint            = true;
                    break;
                }
            }
        }
        if ( !foundPoint )
        {
            if ( isLeftOf( depth, depthIt->second.front(), reverseOrder, eps ) )
            {
                // Extrapolate from front two
                interpolateSegment( resamplingDepthType, depth, 0, xValues, resampledDepths, eps );
                foundPoint = true;
            }
            else if ( isRightOf( depth, depthIt->second.back(), reverseOrder, eps ) )
            {
                // Extrapolate from end two
                const size_t N = depthIt->second.size() - 1;
                interpolateSegment( resamplingDepthType, depth, N - 1, xValues, resampledDepths, eps );
                foundPoint = true;
            }
        }

        CAF_ASSERT( foundPoint );
    }

    reSampledData->setValuesAndDepths( xValues, resampledDepths, m_rkbDiff, m_depthUnit, true );
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

    if ( !m_isExtractionCurve || !m_depths.count( RiaDefines::DepthTypeEnum::MEASURED_DEPTH ) )
    {
        m_intervalsOfContinousValidValues = intervalsOfValidValues;
    }
    else
    {
        size_t intervalsCount = intervalsOfValidValues.size();
        for ( size_t intIdx = 0; intIdx < intervalsCount; intIdx++ )
        {
            std::vector<std::pair<size_t, size_t>> depthValuesIntervals;
            splitIntervalAtEmptySpace( m_depths[RiaDefines::DepthTypeEnum::MEASURED_DEPTH],
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
