/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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
#include "RimRftCrossPlotTools.h"

#include "RifEclipseRftAddress.h"
#include "RifReaderRftInterface.h"

#include "RiaExtractionTools.h"

#include "Well/RigEclipseWellLogExtractor.h"
#include "Well/RigWellPathFormations.h"

#include "Formations/RimWellFormationsFile.h"

#include "RimEclipseResultCase.h"
#include "RimObservedFmuRftData.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"
#include "RimWellPath.h"
#include "RimWellPlotTools.h"

#include <algorithm>
#include <limits>
#include <map>
#include <numeric>

namespace caf
{
template <>
void caf::AppEnum<RimRftCrossPlotTools::DepthFilterMode>::setUp()
{
    addItem( RimRftCrossPlotTools::DepthFilterMode::NONE, "NONE", "None" );
    addItem( RimRftCrossPlotTools::DepthFilterMode::DEPTH_RANGE, "DEPTH_RANGE", "Depth Range" );
    addItem( RimRftCrossPlotTools::DepthFilterMode::ZONES, "ZONES", "Formation" );
    setDefault( RimRftCrossPlotTools::DepthFilterMode::NONE );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimRftCrossPlotTools::depthTypeAbbreviation( RiaDefines::DepthType depthType )
{
    return depthType == RiaDefines::DepthType::TRUE_VERTICAL_DEPTH ? "TVD" : "MD";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimRftCrossPlotTools::rftCurveDepthValues( RifReaderRftInterface*      reader,
                                                               const QString&              wellName,
                                                               const QDateTime&            timeStep,
                                                               RigEclipseWellLogExtractor* extractor,
                                                               RiaDefines::DepthType       depthType )
{
    if ( !reader ) return {};

    if ( depthType == RiaDefines::DepthType::TRUE_VERTICAL_DEPTH )
    {
        auto tvdAddress = RifEclipseRftAddress::createAddress( wellName, timeStep, RifEclipseRftAddress::RftWellLogChannelType::TVD );
        std::vector<double> tvdDepths;
        reader->values( tvdAddress, &tvdDepths );
        return tvdDepths;
    }

    auto mdAddress = RifEclipseRftAddress::createAddress( wellName, timeStep, RifEclipseRftAddress::RftWellLogChannelType::MD );
    std::vector<double> depths;
    reader->values( mdAddress, &depths );
    if ( depths.empty() && extractor ) depths = reader->computeMeasuredDepth( wellName, timeStep, extractor );
    return depths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimRftCrossPlotTools::filterPressuresByDepthRange( const std::vector<double>& depths,
                                                                       const std::vector<double>& pressures,
                                                                       bool                       useDepthRange,
                                                                       double                     depthRangeMin,
                                                                       double                     depthRangeMax )
{
    std::vector<DepthInterval> intervals;
    if ( useDepthRange ) intervals.push_back( { depthRangeMin, depthRangeMax, QString() } );

    return filterPressuresByDepthIntervals( depths, pressures, intervals );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimRftCrossPlotTools::DepthInterval> RimRftCrossPlotTools::buildDepthIntervals( DepthFilterMode             mode,
                                                                                            double                      depthRangeMin,
                                                                                            double                      depthRangeMax,
                                                                                            RimWellFormationsFile*      wellFormationsFile,
                                                                                            const QString&              wellName,
                                                                                            const std::vector<QString>& selectedZones,
                                                                                            RiaDefines::DepthType       depthType )
{
    if ( mode == DepthFilterMode::NONE ) return {};

    if ( mode == DepthFilterMode::DEPTH_RANGE ) return { DepthInterval{ depthRangeMin, depthRangeMax, QString() } };

    // ZONES mode
    if ( !wellFormationsFile || selectedZones.empty() ) return {};

    const RigWellPathFormations* formations = wellFormationsFile->formationsForWell( wellName );
    if ( !formations ) return {};

    std::vector<DepthInterval> intervals;
    for ( size_t i = 0; i < formations->formationCount(); ++i )
    {
        const RigWellPathFormation& formation = formations->formationAt( i );

        bool isSelected = std::find( selectedZones.begin(), selectedZones.end(), formation.formationName ) != selectedZones.end();
        if ( !isSelected ) continue;

        if ( depthType == RiaDefines::DepthType::TRUE_VERTICAL_DEPTH )
            intervals.push_back( { formation.tvdTop, formation.tvdBase, formation.formationName } );
        else
            intervals.push_back( { formation.mdTop, formation.mdBase, formation.formationName } );
    }

    return intervals;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimRftCrossPlotTools::DepthInterval> RimRftCrossPlotTools::buildAllZoneIntervals( RimWellFormationsFile* wellFormationsFile,
                                                                                              const QString&         wellName,
                                                                                              RiaDefines::DepthType  depthType )
{
    if ( !wellFormationsFile ) return {};

    const RigWellPathFormations* formations = wellFormationsFile->formationsForWell( wellName );
    if ( !formations ) return {};

    const bool                 useTvd = depthType == RiaDefines::DepthType::TRUE_VERTICAL_DEPTH;
    std::vector<DepthInterval> intervals;
    for ( size_t i = 0; i < formations->formationCount(); ++i )
    {
        const RigWellPathFormation& formation = formations->formationAt( i );
        intervals.push_back(
            { useTvd ? formation.tvdTop : formation.mdTop, useTvd ? formation.tvdBase : formation.mdBase, formation.formationName } );
    }
    return intervals;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimRftCrossPlotTools::filterPressuresByDepthIntervals( const std::vector<double>&        depths,
                                                                           const std::vector<double>&        pressures,
                                                                           const std::vector<DepthInterval>& depthIntervals )
{
    if ( depthIntervals.empty() ) return pressures;

    // Depth filter requested but no aligned depth data is available; exclude rather than
    // silently return an unfiltered set of samples.
    if ( depths.size() != pressures.size() ) return {};

    std::vector<double> samplesInRange;
    for ( size_t i = 0; i < depths.size(); ++i )
    {
        for ( const DepthInterval& interval : depthIntervals )
        {
            if ( depths[i] >= interval.top && depths[i] <= interval.base )
            {
                samplesInRange.push_back( pressures[i] );
                break;
            }
        }
    }

    return samplesInRange;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimRftCrossPlotTools::depthFilterDescription( DepthFilterMode             mode,
                                                      RiaDefines::DepthType       depthType,
                                                      double                      depthRangeMin,
                                                      double                      depthRangeMax,
                                                      const std::vector<QString>& selectedZones )
{
    if ( mode == DepthFilterMode::NONE ) return {};

    if ( mode == DepthFilterMode::ZONES )
    {
        if ( selectedZones.empty() ) return {};

        QStringList zoneList;
        for ( const QString& zone : selectedZones )
            zoneList << zone;
        return QString( "Zones: %1" ).arg( zoneList.join( ", " ) );
    }

    return QString( "%1 %2 - %3 m" ).arg( depthTypeAbbreviation( depthType ) ).arg( depthRangeMin ).arg( depthRangeMax );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimRftCrossPlotTools::ObservedPressure>
    RimRftCrossPlotTools::computeObservedPressures( const QString&                    wellName,
                                                    const QDateTime&                  timeStep,
                                                    const std::vector<DepthInterval>& depthIntervals,
                                                    RiaDefines::DepthType             depthType,
                                                    const std::vector<DepthInterval>& zoneIntervals )
{
    std::vector<ObservedPressure> result;
    if ( wellName.isEmpty() || !timeStep.isValid() ) return result;

    for ( RimObservedFmuRftData* observedData : RimWellPlotTools::observedFmuRftDataForWell( wellName ) )
    {
        RifReaderRftInterface* reader = observedData->rftReader();
        if ( !reader || !reader->availableTimeSteps( wellName ).count( timeStep ) ) continue;

        auto pressureAddress = RifEclipseRftAddress::createAddress( wellName, timeStep, RifEclipseRftAddress::RftWellLogChannelType::PRESSURE );
        std::vector<double> pressures;
        reader->values( pressureAddress, &pressures );
        if ( pressures.empty() ) continue;

        std::vector<double> depths = rftCurveDepthValues( reader, wellName, timeStep, nullptr, depthType );

        auto errorAddress =
            RifEclipseRftAddress::createAddress( wellName, timeStep, RifEclipseRftAddress::RftWellLogChannelType::PRESSURE_ERROR );
        std::vector<double> errors;
        reader->values( errorAddress, &errors );

        // Filtering pressures and errors with the same depths/intervals keeps the two vectors aligned.
        const std::vector<double> filteredPressures = filterPressuresByDepthIntervals( depths, pressures, depthIntervals );
        std::vector<double>       filteredErrors;
        if ( errors.size() == pressures.size() ) filteredErrors = filterPressuresByDepthIntervals( depths, errors, depthIntervals );

        const std::vector<double> filteredDepths = filterPressuresByDepthIntervals( depths, depths, depthIntervals );

        for ( size_t i = 0; i < filteredPressures.size(); ++i )
        {
            const double error = i < filteredErrors.size() && filteredErrors.size() == filteredPressures.size() ? filteredErrors[i] : 0.0;

            QString zoneName;
            if ( filteredDepths.size() == filteredPressures.size() )
            {
                const auto& lookup    = zoneIntervals.empty() ? depthIntervals : zoneIntervals;
                double      thickness = std::numeric_limits<double>::infinity();
                for ( const auto& interval : lookup )
                {
                    // Prefer the narrowest matching zone when formation levels overlap
                    if ( filteredDepths[i] >= interval.top && filteredDepths[i] <= interval.base && interval.base - interval.top < thickness )
                    {
                        zoneName  = interval.zoneName;
                        thickness = interval.base - interval.top;
                    }
                }
            }
            result.push_back( { filteredPressures[i], error, zoneName, filteredPressures[i] - error, filteredPressures[i] + error } );
        }
    }

    // Combine multiple observations in the same zone: mean pressure, band spanning all observations
    std::vector<ObservedPressure> aggregated;
    std::vector<int>              counts;
    std::map<QString, size_t>     zoneIndex;
    for ( const auto& observed : result )
    {
        if ( observed.zoneName.isEmpty() )
        {
            aggregated.push_back( observed );
            counts.push_back( 1 );
            continue;
        }

        auto [it, inserted] = zoneIndex.insert( { observed.zoneName, aggregated.size() } );
        if ( inserted )
        {
            aggregated.push_back( observed );
            counts.push_back( 1 );
            continue;
        }

        ObservedPressure& target = aggregated[it->second];
        target.pressure += observed.pressure;
        target.rangeMin = std::min( target.rangeMin, observed.rangeMin );
        target.rangeMax = std::max( target.rangeMax, observed.rangeMax );
        counts[it->second]++;
        target.count++;
    }

    for ( size_t i = 0; i < aggregated.size(); ++i )
    {
        aggregated[i].pressure /= counts[i];
        aggregated[i].error = std::max( aggregated[i].pressure - aggregated[i].rangeMin, aggregated[i].rangeMax - aggregated[i].pressure );
    }

    return aggregated;
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimRftCrossPlotTools::computeMean( const std::vector<double>& samples )
{
    if ( samples.empty() ) return std::numeric_limits<double>::infinity();

    return std::accumulate( samples.begin(), samples.end(), 0.0 ) / samples.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<double>> RimRftCrossPlotTools::computePressureSamplesPerCase( RimSummaryEnsemble*               ensemble,
                                                                                      const QString&                    wellName,
                                                                                      const QDateTime&                  timeStep,
                                                                                      RimEclipseResultCase*             eclipseCase,
                                                                                      const std::vector<DepthInterval>& depthIntervals,
                                                                                      RiaDefines::DepthType             depthType )
{
    if ( !ensemble || wellName.isEmpty() || !timeStep.isValid() ) return {};

    RigEclipseWellLogExtractor* extractor = nullptr;
    if ( eclipseCase )
    {
        RimWellPath* wellPath = RimProject::current()->wellPathFromSimWellName( wellName );
        extractor             = RiaExtractionTools::findOrCreateWellLogExtractor( wellPath, eclipseCase );
        if ( !extractor ) extractor = RiaExtractionTools::findOrCreateSimWellExtractor( eclipseCase, wellName, false, 0 );
    }

    const auto& allCases = ensemble->allSummaryCases();

    std::vector<std::vector<double>> samplesPerCase;
    samplesPerCase.reserve( allCases.size() );

    for ( RimSummaryCase* summaryCase : allCases )
    {
        if ( !summaryCase )
        {
            samplesPerCase.push_back( {} );
            continue;
        }

        RifReaderRftInterface* reader = summaryCase->rftReader();
        if ( !reader )
        {
            samplesPerCase.push_back( {} );
            continue;
        }

        auto pressureAddress = RifEclipseRftAddress::createAddress( wellName, timeStep, RifEclipseRftAddress::RftWellLogChannelType::PRESSURE );
        std::vector<double> pressures;
        reader->values( pressureAddress, &pressures );
        if ( pressures.empty() )
        {
            samplesPerCase.push_back( {} );
            continue;
        }

        // Use the same depth values the RFT curves use for their depth axis, so the filter
        // operates on values consistent with what the user sees in the RFT plot.
        std::vector<double> depths = rftCurveDepthValues( reader, wellName, timeStep, extractor, depthType );

        samplesPerCase.push_back( filterPressuresByDepthIntervals( depths, pressures, depthIntervals ) );
    }

    return samplesPerCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimRftCrossPlotTools::computeMeanPressurePerCase( RimSummaryEnsemble*               ensemble,
                                                                      const QString&                    wellName,
                                                                      const QDateTime&                  timeStep,
                                                                      RimEclipseResultCase*             eclipseCase,
                                                                      const std::vector<DepthInterval>& depthIntervals,
                                                                      RiaDefines::DepthType             depthType )
{
    const std::vector<std::vector<double>> samplesPerCase =
        computePressureSamplesPerCase( ensemble, wellName, timeStep, eclipseCase, depthIntervals, depthType );

    std::vector<double> pressurePerCase;
    pressurePerCase.reserve( samplesPerCase.size() );

    for ( const std::vector<double>& samples : samplesPerCase )
        pressurePerCase.push_back( computeMean( samples ) );

    return pressurePerCase;
}
