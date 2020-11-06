/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RifReaderEnsembleStatisticsRft.h"

#include "RiaCurveMerger.h"
#include "RiaWeightedMeanCalculator.h"
#include "RigStatisticsMath.h"

#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"

#include "cafAssert.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderEnsembleStatisticsRft::RifReaderEnsembleStatisticsRft( const RimSummaryCaseCollection* summaryCaseCollection )
    : m_summaryCaseCollection( summaryCaseCollection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseRftAddress> RifReaderEnsembleStatisticsRft::eclipseRftAddresses()
{
    std::set<RifEclipseRftAddress> allAddresses;
    for ( auto summaryCase : m_summaryCaseCollection->allSummaryCases() )
    {
        if ( summaryCase->rftReader() )
        {
            std::set<RifEclipseRftAddress> addresses = summaryCase->rftReader()->eclipseRftAddresses();
            allAddresses.insert( addresses.begin(), addresses.end() );
        }
    }

    std::set<RifEclipseRftAddress> statisticsAddresses;
    for ( const RifEclipseRftAddress& regularAddress : allAddresses )
    {
        if ( regularAddress.wellLogChannel() == RifEclipseRftAddress::TVD )
        {
            statisticsAddresses.insert( regularAddress );
        }
        else if ( regularAddress.wellLogChannel() == RifEclipseRftAddress::PRESSURE )
        {
            std::set<RifEclipseRftAddress::RftWellLogChannelType> statChannels = { RifEclipseRftAddress::PRESSURE_P10,
                                                                                   RifEclipseRftAddress::PRESSURE_P50,
                                                                                   RifEclipseRftAddress::PRESSURE_P90,
                                                                                   RifEclipseRftAddress::PRESSURE_MEAN };
            for ( auto channel : statChannels )
            {
                statisticsAddresses.insert(
                    RifEclipseRftAddress( regularAddress.wellName(), regularAddress.timeStep(), channel ) );
            }
        }
    }
    return statisticsAddresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderEnsembleStatisticsRft::values( const RifEclipseRftAddress& rftAddress, std::vector<double>* values )
{
    CAF_ASSERT( rftAddress.wellLogChannel() == RifEclipseRftAddress::TVD ||
                rftAddress.wellLogChannel() == RifEclipseRftAddress::PRESSURE_MEAN ||
                rftAddress.wellLogChannel() == RifEclipseRftAddress::PRESSURE_P10 ||
                rftAddress.wellLogChannel() == RifEclipseRftAddress::PRESSURE_P50 ||
                rftAddress.wellLogChannel() == RifEclipseRftAddress::PRESSURE_P90 ||
                rftAddress.wellLogChannel() == RifEclipseRftAddress::PRESSURE_ERROR );

    auto it = m_cachedValues.find( rftAddress );
    if ( it == m_cachedValues.end() )
    {
        calculateStatistics( rftAddress );
    }
    *values = m_cachedValues[rftAddress];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QDateTime> RifReaderEnsembleStatisticsRft::availableTimeSteps( const QString& wellName )
{
    std::set<QDateTime> allTimeSteps;
    for ( auto summaryCase : m_summaryCaseCollection->allSummaryCases() )
    {
        if ( summaryCase->rftReader() )
        {
            std::set<QDateTime> timeSteps = summaryCase->rftReader()->availableTimeSteps( wellName );
            allTimeSteps.insert( timeSteps.begin(), timeSteps.end() );
        }
    }
    return allTimeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QDateTime>
    RifReaderEnsembleStatisticsRft::availableTimeSteps( const QString&                                     wellName,
                                                        const RifEclipseRftAddress::RftWellLogChannelType& wellLogChannelName )
{
    std::set<QDateTime> allTimeSteps;
    for ( auto summaryCase : m_summaryCaseCollection->allSummaryCases() )
    {
        if ( summaryCase->rftReader() )
        {
            std::set<QDateTime> timeSteps = summaryCase->rftReader()->availableTimeSteps( wellName, wellLogChannelName );
            allTimeSteps.insert( timeSteps.begin(), timeSteps.end() );
        }
    }
    return allTimeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QDateTime> RifReaderEnsembleStatisticsRft::availableTimeSteps(
    const QString&                                               wellName,
    const std::set<RifEclipseRftAddress::RftWellLogChannelType>& relevantChannels )
{
    std::set<QDateTime> allTimeSteps;
    for ( auto summaryCase : m_summaryCaseCollection->allSummaryCases() )
    {
        if ( summaryCase->rftReader() )
        {
            std::set<QDateTime> timeSteps = summaryCase->rftReader()->availableTimeSteps( wellName, relevantChannels );
            allTimeSteps.insert( timeSteps.begin(), timeSteps.end() );
        }
    }
    return allTimeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseRftAddress::RftWellLogChannelType>
    RifReaderEnsembleStatisticsRft::availableWellLogChannels( const QString& wellName )
{
    std::set<RifEclipseRftAddress::RftWellLogChannelType> allWellLogChannels;
    for ( auto summaryCase : m_summaryCaseCollection->allSummaryCases() )
    {
        if ( summaryCase->rftReader() )
        {
            std::set<RifEclipseRftAddress::RftWellLogChannelType> wellLogChannels =
                summaryCase->rftReader()->availableWellLogChannels( wellName );
            allWellLogChannels.insert( wellLogChannels.begin(), wellLogChannels.end() );
        }
    }
    return allWellLogChannels;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RifReaderEnsembleStatisticsRft::wellNames()
{
    std::set<QString> allWellNames;
    for ( auto summaryCase : m_summaryCaseCollection->allSummaryCases() )
    {
        if ( summaryCase->rftReader() )
        {
            std::set<QString> wellNames = summaryCase->rftReader()->wellNames();
            allWellNames.insert( wellNames.begin(), wellNames.end() );
        }
    }
    return allWellNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderEnsembleStatisticsRft::calculateStatistics( const RifEclipseRftAddress& rftAddress )
{
    const QString&       wellName = rftAddress.wellName();
    const QDateTime&     timeStep = rftAddress.timeStep();
    RifEclipseRftAddress depthAddress( wellName, timeStep, RifEclipseRftAddress::TVD );
    RifEclipseRftAddress pressAddress( wellName, timeStep, RifEclipseRftAddress::PRESSURE );

    RifEclipseRftAddress p10Address( wellName, timeStep, RifEclipseRftAddress::PRESSURE_P10 );
    RifEclipseRftAddress p50Address( wellName, timeStep, RifEclipseRftAddress::PRESSURE_P50 );
    RifEclipseRftAddress p90Address( wellName, timeStep, RifEclipseRftAddress::PRESSURE_P90 );
    RifEclipseRftAddress meanAddress( wellName, timeStep, RifEclipseRftAddress::PRESSURE_MEAN );

    RiaCurveMerger<double> curveMerger;

    RiaWeightedMeanCalculator<size_t> dataSetSizeCalc;

    for ( RimSummaryCase* summaryCase : m_summaryCaseCollection->allSummaryCases() )
    {
        RifReaderRftInterface* reader = summaryCase->rftReader();
        if ( reader )
        {
            std::vector<double> depths;
            std::vector<double> pressures;
            reader->values( depthAddress, &depths );
            reader->values( pressAddress, &pressures );
            dataSetSizeCalc.addValueAndWeight( depths.size(), 1.0 );
            curveMerger.addCurveData( depths, pressures );
        }
    }
    curveMerger.computeInterpolatedValues( false );

    clearData( wellName, timeStep );

    const std::vector<double>& allDepths = curveMerger.allXValues();
    if ( !allDepths.empty() )
    {
        // Make sure we end up with approximately the same amount of points as originally
        // Since allDepths contain *valid* values, it can potentially be smaller than the mean.
        // Thus we need to ensure sizeMultiplier is at least 1.
        size_t sizeMultiplier = std::max( (size_t)1, allDepths.size() / dataSetSizeCalc.weightedMean() );
        for ( size_t depthIdx = 0; depthIdx < allDepths.size(); depthIdx += sizeMultiplier )
        {
            std::vector<double> pressuresAtDepth;
            pressuresAtDepth.reserve( curveMerger.curveCount() );
            for ( size_t curveIdx = 0; curveIdx < curveMerger.curveCount(); ++curveIdx )
            {
                const std::vector<double>& curvePressures = curveMerger.interpolatedYValuesForAllXValues( curveIdx );
                pressuresAtDepth.push_back( curvePressures[depthIdx] );
            }
            double p10, p50, p90, mean;
            RigStatisticsMath::calculateStatisticsCurves( pressuresAtDepth, &p10, &p50, &p90, &mean );

            m_cachedValues[depthAddress].push_back( allDepths[depthIdx] );

            if ( p10 != HUGE_VAL ) m_cachedValues[p10Address].push_back( p10 );
            if ( p50 != HUGE_VAL ) m_cachedValues[p50Address].push_back( p50 );
            if ( p90 != HUGE_VAL ) m_cachedValues[p90Address].push_back( p90 );
            m_cachedValues[meanAddress].push_back( mean );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderEnsembleStatisticsRft::clearData( const QString& wellName, const QDateTime& timeStep )
{
    for ( auto it = m_cachedValues.begin(); it != m_cachedValues.end(); )
    {
        if ( it->first.wellName() == wellName && it->first.timeStep() == timeStep )
        {
            it = m_cachedValues.erase( it );
        }
        else
        {
            ++it;
        }
    }
}
