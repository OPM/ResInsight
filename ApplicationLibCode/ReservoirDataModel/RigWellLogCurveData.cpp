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
#include "RiaWellLogUnitTools.h"

#include "cvfAssert.h"
#include "cvfMath.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellLogCurveData::RigWellLogCurveData()
    : m_isExtractionCurve( false )
    , m_rkbDiff( 0.0 )
    , m_useLogarithmicScale( false )
    , m_depthUnit( RiaDefines::DepthUnitType::UNIT_METER )
    , m_propertyValueUnitString( RiaWellLogUnitTools<double>::noUnitString() )

{
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
void RigWellLogCurveData::clear()
{
    m_propertyValues.clear();
    m_depths.clear();
    m_intervalsOfContinousValidValues.clear();
    m_propertyValueUnitString.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellLogCurveData::setDepthUnit( RiaDefines::DepthUnitType depthUnit )
{
    m_depthUnit = depthUnit;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellLogCurveData::setValuesAndDepths( const std::vector<double>& xValues,
                                              const std::vector<double>& depths,
                                              RiaDefines::DepthTypeEnum  depthType,
                                              double                     rkbDiff,
                                              RiaDefines::DepthUnitType  depthUnit,
                                              bool                       isExtractionCurve,
                                              bool                       useLogarithmicScale )
{
    CVF_ASSERT( xValues.size() == depths.size() );

    m_propertyValues      = xValues;
    m_depths[depthType]   = depths;
    m_depthUnit           = depthUnit;
    m_rkbDiff             = rkbDiff;
    m_useLogarithmicScale = useLogarithmicScale;

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
                                              bool                                                            isExtractionCurve,
                                              bool                                                            useLogarithmicScale )
{
    for ( auto it = depths.begin(); it != depths.end(); ++it )
    {
        CVF_ASSERT( xValues.size() == it->second.size() );
    }

    m_propertyValues      = xValues;
    m_depths              = depths;
    m_depthUnit           = depthUnit;
    m_rkbDiff             = rkbDiff;
    m_useLogarithmicScale = useLogarithmicScale;

    // Disable depth value filtering is intended to be used for
    // extraction curve data
    m_isExtractionCurve = isExtractionCurve;

    calculateIntervalsOfContinousValidValues();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellLogCurveData::setPropertyValueUnit( const QString& propertyValueUnitString )
{
    m_propertyValueUnitString = propertyValueUnitString;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellLogCurveData::propertyValues() const
{
    return m_propertyValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellLogCurveData::propertyValues( const QString& units ) const
{
    std::vector<double> convertedValues;
    if ( units != m_propertyValueUnitString &&
         RiaWellLogUnitTools<double>::convertValues( depths( RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH_RKB ),
                                                     m_propertyValues,
                                                     &convertedValues,
                                                     m_propertyValueUnitString,
                                                     units ) )
    {
        return convertedValues;
    }
    return m_propertyValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigWellLogCurveData::propertyValueUnit() const
{
    return m_propertyValueUnitString;
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

    if ( depthType == RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH_RKB && m_depths.count( RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH ) )
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
std::vector<double> RigWellLogCurveData::depths( RiaDefines::DepthTypeEnum depthType, RiaDefines::DepthUnitType destinationDepthUnit ) const
{
    return depthsForDepthUnit( depths( depthType ), m_depthUnit, destinationDepthUnit );
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
std::vector<double> RigWellLogCurveData::propertyValuesByIntervals() const
{
    std::vector<double> filteredValues;
    RiaCurveDataTools::getValuesByIntervals( m_propertyValues, m_intervalsOfContinousValidValues, &filteredValues );

    return filteredValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellLogCurveData::depthValuesByIntervals( RiaDefines::DepthTypeEnum depthType,
                                                                 RiaDefines::DepthUnitType destinationDepthUnit ) const
{
    const std::vector<double> depthValues = RigWellLogCurveData::depthsForDepthUnit( depths( depthType ), m_depthUnit, destinationDepthUnit );

    std::vector<double> filteredValues;
    RiaCurveDataTools::getValuesByIntervals( depthValues, m_intervalsOfContinousValidValues, &filteredValues );

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
            double segmentStartX  = m_propertyValues[segmentStartIdx];
            double segmentEndX    = m_propertyValues[segmentStartIdx + 1];

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
            { { RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH, tvDepths }, { RiaDefines::DepthTypeEnum::MEASURED_DEPTH, measuredDepths } };
        reSampledData->setValuesAndDepths( xValues, resampledDepths, m_rkbDiff, m_depthUnit, true, m_useLogarithmicScale );
    }
    else
    {
        reSampledData->setValuesAndDepths( xValues,
                                           measuredDepths,
                                           RiaDefines::DepthTypeEnum::MEASURED_DEPTH,
                                           0.0,
                                           m_depthUnit,
                                           m_isExtractionCurve,
                                           m_useLogarithmicScale );
    }

    return reSampledData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellLogCurveData::interpolateSegment( RiaDefines::DepthTypeEnum                                       resamplingDepthType,
                                              std::vector<double>&                                            resampledValues,
                                              std::map<RiaDefines::DepthTypeEnum, std::vector<double>>&       resampledDepths,
                                              double                                                          targetDepthValue,
                                              size_t                                                          firstIndex,
                                              const std::map<RiaDefines::DepthTypeEnum, std::vector<double>>& originalDepths,
                                              const std::vector<double>&                                      propertyValues,
                                              double                                                          eps )
{
    if ( !originalDepths.contains( resamplingDepthType ) ) return;

    const size_t secondIndex = firstIndex + 1;
    const auto&  depthValues = originalDepths.find( resamplingDepthType )->second;
    if ( secondIndex >= depthValues.size() ) return;

    const double depth0 = depthValues[firstIndex];
    const double depth1 = depthValues[secondIndex];
    const double x0     = propertyValues[firstIndex];
    const double x1     = propertyValues[secondIndex];
    double       slope  = 0.0;
    if ( std::fabs( depth1 - depth0 ) > eps )
    {
        slope = ( x1 - x0 ) / ( depth1 - depth0 );
    }
    const double resampledValue = slope * ( targetDepthValue - depth0 ) + x0;
    resampledValues.push_back( resampledValue );

    for ( const auto& [depthType, depthTypeValues] : originalDepths )
    {
        // Skip the depth type we are resampling and ensure depth values are available
        if ( depthType == resamplingDepthType ) continue;
        if ( depthTypeValues.size() < secondIndex - 1 ) continue;

        const double otherDepth0 = depthTypeValues[firstIndex];
        const double otherDepth1 = depthTypeValues[secondIndex];
        const double otherSlope  = ( otherDepth1 - otherDepth0 ) / ( depth1 - depth0 );
        resampledDepths[depthType].push_back( otherSlope * ( targetDepthValue - depth0 ) + otherDepth0 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool isLeftOf( double x1, double x2, bool reverseOrder, double eps )
{
    if ( reverseOrder )
    {
        return x1 - x2 > eps;
    }
    return x2 - x1 > eps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool isRightOf( double x1, double x2, bool reverseOrder, double eps )
{
    return isLeftOf( x2, x1, reverseOrder, eps );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::vector<double>, std::map<RiaDefines::DepthTypeEnum, std::vector<double>>>
    RigWellLogCurveData::createResampledValuesAndDepths( RiaDefines::DepthTypeEnum  resamplingDepthType,
                                                         const std::vector<double>& targetDepths,
                                                         const std::map<RiaDefines::DepthTypeEnum, std::vector<double>>& originalDepths,
                                                         const std::vector<double>&                                      propertyValues )
{
    const double eps = 1.0e-8;

    auto depthIt = originalDepths.find( resamplingDepthType );
    if ( depthIt == originalDepths.end() || depthIt->second.empty() ) return {};
    const auto& depthValues = depthIt->second;

    std::vector<double>                                      resampledValues;
    std::map<RiaDefines::DepthTypeEnum, std::vector<double>> resampledDepths;
    resampledDepths.insert( std::make_pair( resamplingDepthType, targetDepths ) );

    cvf::ref<RigWellLogCurveData> resampledCurveData = new RigWellLogCurveData;

    bool reverseOrder = resamplingDepthType == RiaDefines::DepthTypeEnum::CONNECTION_NUMBER;

    size_t segmentSearchStartIdx = 0;
    for ( const auto& targetDepth : targetDepths )
    {
        bool foundPoint = false;
        for ( size_t segmentStartIdx = segmentSearchStartIdx; segmentStartIdx < depthValues.size(); ++segmentStartIdx )
        {
            if ( std::fabs( depthValues[segmentStartIdx] - targetDepth ) < eps ) // already have this depth point, reuse it
            {
                resampledValues.push_back( propertyValues[segmentStartIdx] );
                // Copy all depth types for this segment
                for ( const auto& depthTypeValuesPair : originalDepths )
                {
                    if ( depthTypeValuesPair.first != resamplingDepthType )
                    {
                        resampledDepths[depthTypeValuesPair.first].push_back( depthTypeValuesPair.second[segmentStartIdx] );
                    }
                }
                segmentSearchStartIdx = segmentStartIdx;
                foundPoint            = true;
                break;
            }
            else if ( segmentStartIdx < depthValues.size() - 1 )
            {
                double minDepthSegment = std::min( depthValues[segmentStartIdx], depthValues[segmentStartIdx + 1] );
                double maxDepthSegment = std::max( depthValues[segmentStartIdx], depthValues[segmentStartIdx + 1] );
                if ( cvf::Math::valueInRange( targetDepth, minDepthSegment, maxDepthSegment ) )
                {
                    interpolateSegment( resamplingDepthType,
                                        resampledValues,
                                        resampledDepths,
                                        targetDepth,
                                        segmentStartIdx,
                                        originalDepths,
                                        propertyValues,
                                        eps );
                    segmentSearchStartIdx = segmentStartIdx;
                    foundPoint            = true;
                    break;
                }
            }
        }
        if ( !foundPoint )
        {
            if ( isLeftOf( targetDepth, depthValues.front(), reverseOrder, eps ) )
            {
                // Extrapolate from front two
                const size_t firstIndex = 0;
                interpolateSegment( resamplingDepthType, resampledValues, resampledDepths, targetDepth, firstIndex, originalDepths, propertyValues, eps );
                foundPoint = true;
            }
            else if ( isRightOf( targetDepth, depthValues.back(), reverseOrder, eps ) )
            {
                // Extrapolate from end two
                const size_t N = depthValues.size() - 1;
                interpolateSegment( resamplingDepthType, resampledValues, resampledDepths, targetDepth, N - 1, originalDepths, propertyValues, eps );
                foundPoint = true;
            }
        }

        CAF_ASSERT( foundPoint );
    }

    return std::make_pair( resampledValues, resampledDepths );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigWellLogCurveData> RigWellLogCurveData::calculateResampledCurveData( RiaDefines::DepthTypeEnum  resamplingDepthType,
                                                                                const std::vector<double>& depths ) const
{
    const auto [xValues, resampledDepths] = createResampledValuesAndDepths( resamplingDepthType, depths, m_depths, m_propertyValues );

    cvf::ref<RigWellLogCurveData> reSampledData = new RigWellLogCurveData;
    reSampledData->setValuesAndDepths( xValues, resampledDepths, m_rkbDiff, m_depthUnit, true, m_useLogarithmicScale );
    return reSampledData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellLogCurveData::calculateIntervalsOfContinousValidValues()
{
    std::vector<std::pair<size_t, size_t>> intervalsOfValidValues =
        RiaCurveDataTools::calculateIntervalsOfValidValues( m_propertyValues, m_useLogarithmicScale );

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
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RigWellLogCurveData::depthsForDepthUnit( const std::vector<double>& depths,
                                                             RiaDefines::DepthUnitType  sourceDepthUnit,
                                                             RiaDefines::DepthUnitType  destinationDepthUnit )
{
    if ( destinationDepthUnit == sourceDepthUnit ) return depths;

    std::vector<double> convertedValues = RiaWellLogUnitTools<double>::convertDepths( depths, sourceDepthUnit, destinationDepthUnit );
    return convertedValues;
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

    std::vector<double> depthValues = depthValuesByIntervals( depthType, depthUnit );
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
