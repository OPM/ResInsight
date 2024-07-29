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

#include "RiaExtractionTools.h"

#include "RigEclipseWellLogExtractor.h"
#include "RigStatisticsMath.h"
#include "RigWellPath.h"

#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"
#include "RimTools.h"

#include "cafAssert.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderEnsembleStatisticsRft::RifReaderEnsembleStatisticsRft( const RimSummaryEnsemble* summaryCaseCollection, RimEclipseCase* eclipseCase )
    : m_summaryCaseCollection( summaryCaseCollection )
    , m_eclipseCase( eclipseCase )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseRftAddress> RifReaderEnsembleStatisticsRft::eclipseRftAddresses()
{
    if ( !m_summaryCaseCollection ) return {};

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
        if ( regularAddress.wellLogChannel() == RifEclipseRftAddress::RftWellLogChannelType::TVD )
        {
            statisticsAddresses.insert( regularAddress );
        }
        else if ( regularAddress.wellLogChannel() == RifEclipseRftAddress::RftWellLogChannelType::PRESSURE )
        {
            std::set<RifEclipseRftAddress::RftWellLogChannelType> statChannels = { RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_P10,
                                                                                   RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_P50,
                                                                                   RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_P90,
                                                                                   RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_MEAN };
            for ( auto channel : statChannels )
            {
                statisticsAddresses.insert(
                    RifEclipseRftAddress::createAddress( regularAddress.wellName(), regularAddress.timeStep(), channel ) );
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
    if ( !m_summaryCaseCollection ) return;

    CAF_ASSERT( rftAddress.wellLogChannel() == RifEclipseRftAddress::RftWellLogChannelType::MD ||
                rftAddress.wellLogChannel() == RifEclipseRftAddress::RftWellLogChannelType::TVD ||
                rftAddress.wellLogChannel() == RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_MEAN ||
                rftAddress.wellLogChannel() == RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_P10 ||
                rftAddress.wellLogChannel() == RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_P50 ||
                rftAddress.wellLogChannel() == RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_P90 ||
                rftAddress.wellLogChannel() == RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_ERROR );

    auto it = m_cachedValues.find( rftAddress );
    if ( it == m_cachedValues.end() )
    {
        calculateStatistics( rftAddress.wellName(), rftAddress.timeStep() );
    }
    *values = m_cachedValues[rftAddress];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QDateTime> RifReaderEnsembleStatisticsRft::availableTimeSteps( const QString& wellName )
{
    if ( !m_summaryCaseCollection ) return {};

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
std::set<QDateTime> RifReaderEnsembleStatisticsRft::availableTimeSteps( const QString& wellName,
                                                                        const RifEclipseRftAddress::RftWellLogChannelType& wellLogChannelName )
{
    if ( !m_summaryCaseCollection ) return {};

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
std::set<QDateTime>
    RifReaderEnsembleStatisticsRft::availableTimeSteps( const QString&                                               wellName,
                                                        const std::set<RifEclipseRftAddress::RftWellLogChannelType>& relevantChannels )
{
    if ( !m_summaryCaseCollection ) return {};

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
std::set<RifEclipseRftAddress::RftWellLogChannelType> RifReaderEnsembleStatisticsRft::availableWellLogChannels( const QString& wellName )
{
    if ( !m_summaryCaseCollection ) return {};

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
    if ( !m_summaryCaseCollection ) return {};

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
void RifReaderEnsembleStatisticsRft::calculateStatistics( const QString& wellName, const QDateTime& timeStep )
{
    if ( !m_summaryCaseCollection ) return;

    using ChannelType                 = RifEclipseRftAddress::RftWellLogChannelType;
    RifEclipseRftAddress pressAddress = RifEclipseRftAddress::createAddress( wellName, timeStep, ChannelType::PRESSURE );
    RifEclipseRftAddress tvdAddress   = RifEclipseRftAddress::createAddress( wellName, timeStep, ChannelType::TVD );

    RigEclipseWellLogExtractor* extractor = RiaExtractionTools::findOrCreateWellLogExtractor( wellName, m_eclipseCase );
    if ( extractor )
    {
        // Create a well log extractor if a well path and an Eclipse case is defined
        // Use the extractor to compute measured depth for RFT cells
        // The TVD values is extracted from the first summary case

        RifEclipseRftAddress              mdAddress = RifEclipseRftAddress::createAddress( wellName, timeStep, ChannelType::MD );
        RiaCurveMerger<double>            curveMerger;
        RiaWeightedMeanCalculator<size_t> dataSetSizeCalc;

        for ( RimSummaryCase* summaryCase : m_summaryCaseCollection->allSummaryCases() )
        {
            auto reader = summaryCase->rftReader();
            if ( !reader ) continue;

            std::vector<double> pressures;
            reader->values( pressAddress, &pressures );

            std::vector<double> measuredDepths = reader->computeMeasuredDepth( wellName, timeStep, extractor );

            if ( !measuredDepths.empty() && !pressures.empty() )
            {
                dataSetSizeCalc.addValueAndWeight( measuredDepths.size(), 1.0 );
                curveMerger.addCurveData( measuredDepths, pressures );
            }
        }

        extractStatisticsFromCurveMerger( wellName, timeStep, mdAddress, curveMerger, dataSetSizeCalc, extractor->wellPathGeometry() );
    }
    else
    {
        // Compute statistics based on TVD depths. No measured depth can be estimated.
        // This concept works well for vertical wells, but does not work for horizontal wells.

        RiaCurveMerger<double>            curveMerger;
        RiaWeightedMeanCalculator<size_t> dataSetSizeCalc;
        RifEclipseRftAddress              tvdAddress = RifEclipseRftAddress::createAddress( wellName, timeStep, ChannelType::TVD );

        for ( RimSummaryCase* summaryCase : m_summaryCaseCollection->allSummaryCases() )
        {
            auto reader = summaryCase->rftReader();
            if ( !reader ) continue;

            std::vector<double> pressures;
            reader->values( pressAddress, &pressures );

            std::vector<double> tvdDepths;
            reader->values( tvdAddress, &tvdDepths );

            if ( !tvdDepths.empty() && !pressures.empty() )
            {
                dataSetSizeCalc.addValueAndWeight( tvdDepths.size(), 1.0 );
                curveMerger.addCurveData( tvdDepths, pressures );
            }
        }

        extractStatisticsFromCurveMerger( wellName, timeStep, tvdAddress, curveMerger, dataSetSizeCalc, nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
/// Compute statistics for values, either based on measured depth or TVD
//--------------------------------------------------------------------------------------------------
void RifReaderEnsembleStatisticsRft::extractStatisticsFromCurveMerger( const QString&                     wellName,
                                                                       const QDateTime&                   timeStep,
                                                                       RifEclipseRftAddress               depthAddress,
                                                                       RiaCurveMerger<double>&            curveMerger,
                                                                       RiaWeightedMeanCalculator<size_t>& dataSetSizeCalc,
                                                                       const RigWellPath*                 wellPathGeometry )
{
    using ChannelType = RifEclipseRftAddress::RftWellLogChannelType;

    CAF_ASSERT( depthAddress.wellLogChannel() == ChannelType::MD || depthAddress.wellLogChannel() == ChannelType::TVD );

    auto p10Address  = RifEclipseRftAddress::createAddress( wellName, timeStep, ChannelType::PRESSURE_P10 );
    auto p50Address  = RifEclipseRftAddress::createAddress( wellName, timeStep, ChannelType::PRESSURE_P50 );
    auto p90Address  = RifEclipseRftAddress::createAddress( wellName, timeStep, ChannelType::PRESSURE_P90 );
    auto meanAddress = RifEclipseRftAddress::createAddress( wellName, timeStep, ChannelType::PRESSURE_MEAN );

    clearCache( wellName, timeStep );

    curveMerger.computeInterpolatedValues( false );

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
            RigStatisticsMath::calculateStatisticsCurves( pressuresAtDepth, &p10, &p50, &p90, &mean, RigStatisticsMath::PercentileStyle::SWITCHED );

            m_cachedValues[depthAddress].push_back( allDepths[depthIdx] );

            if ( wellPathGeometry && ( depthAddress.wellLogChannel() == ChannelType::MD ) )
            {
                // Compute corresponding TVD for given MD
                auto tvdAddress = RifEclipseRftAddress::createAddress( wellName, timeStep, ChannelType::TVD );
                auto tvdDepth   = wellPathGeometry->interpolatedPointAlongWellPath( allDepths[depthIdx] );
                m_cachedValues[tvdAddress].push_back( -tvdDepth.z() );
            }

            if ( p10 != HUGE_VAL ) m_cachedValues[p10Address].push_back( p10 );
            if ( p50 != HUGE_VAL ) m_cachedValues[p50Address].push_back( p50 );
            if ( p90 != HUGE_VAL ) m_cachedValues[p90Address].push_back( p90 );
            if ( mean != HUGE_VAL ) m_cachedValues[meanAddress].push_back( mean );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderEnsembleStatisticsRft::clearCache( const QString& wellName, const QDateTime& timeStep )
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
