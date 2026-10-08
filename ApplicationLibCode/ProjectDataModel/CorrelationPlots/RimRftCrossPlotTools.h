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
#pragma once

#include "RiaDefines.h"

#include "cafAppEnum.h"

#include <QDateTime>
#include <QString>
#include <QStringList>

#include <optional>
#include <vector>

class RifReaderRftInterface;
class RigEclipseWellLogExtractor;
class RimEclipseResultCase;
class RimSummaryEnsemble;
class RimWellFormationsFile;

//==================================================================================================
///
/// Helpers for filtering and reducing RFT pressure data by depth range, shared by the RFT
/// correlation parameter cross plot (RimParameterRftCrossPlot) and tornado plot (RimRftTornadoPlot).
///
//==================================================================================================
namespace RimRftCrossPlotTools
{
// How the active depth filter selects samples: a single depth range, or a set of named zones
// looked up in a well formations file.
enum class DepthFilterMode
{
    NONE,
    DEPTH_RANGE,
    ZONES
};
using DepthFilterModeEnum = caf::AppEnum<DepthFilterMode>;

// A single inclusive depth interval, in the unit/datum of whichever depth type it was built for.
struct DepthInterval
{
    double  top  = 0.0;
    double  base = 0.0;
    QString zoneName; // set when the interval comes from a formation zone
};

// Short abbreviation used in axis titles and plot titles for the active depth type.
QString depthTypeAbbreviation( RiaDefines::DepthType depthType );

// Returns the depth values for the given well/time step, strictly matching depthType: MD
// (native RFT MD channel, falling back to extractor-derived MD if the channel is missing) or
// TVD (native RFT TVD channel). Returns an empty vector if the requested depth type is not
// available, rather than silently substituting the other depth type — callers that filter by
// depth range rely on this to exclude/blank out data instead of comparing a range against the
// wrong unit.
std::vector<double> rftCurveDepthValues( RifReaderRftInterface*      reader,
                                         const QString&              wellName,
                                         const QDateTime&            timeStep,
                                         RigEclipseWellLogExtractor* extractor,
                                         RiaDefines::DepthType       depthType = RiaDefines::DepthType::MEASURED_DEPTH );

// Returns the subset of pressures whose corresponding depth is within [depthRangeMin, depthRangeMax].
// If useDepthRange is false, all pressures are returned unfiltered. If useDepthRange is true and
// depths/pressures are not the same size, an empty vector is returned (no aligned depth data).
std::vector<double> filterPressuresByDepthRange( const std::vector<double>& depths,
                                                 const std::vector<double>& pressures,
                                                 bool                       useDepthRange,
                                                 double                     depthRangeMin,
                                                 double                     depthRangeMax );

// Builds the active depth filter intervals from either a depth range or a set of selected zones.
// Returns an empty vector (meaning "no filter") when the mode is NONE, or when ZONES mode is
// selected but no well formations file, well name, or matching zones are available.
std::vector<DepthInterval> buildDepthIntervals( DepthFilterMode             mode,
                                                double                      depthRangeMin,
                                                double                      depthRangeMax,
                                                RimWellFormationsFile*      wellFormationsFile,
                                                const QString&              wellName,
                                                const std::vector<QString>& selectedZones,
                                                RiaDefines::DepthType       depthType );

// Returns one interval per formation zone of the well (named by zone), used to look up which zone a
// depth belongs to. Empty if no well formations file or no formations for the well.
std::vector<DepthInterval>
    buildAllZoneIntervals( RimWellFormationsFile* wellFormationsFile, const QString& wellName, RiaDefines::DepthType depthType );

// Returns the subset of pressures whose corresponding depth lies within any of depthIntervals
// (inclusive). An empty depthIntervals list means "no filter", returning pressures unchanged; a
// size mismatch between depths/pressures returns an empty vector (no aligned depth data).
std::vector<double> filterPressuresByDepthIntervals( const std::vector<double>&        depths,
                                                     const std::vector<double>&        pressures,
                                                     const std::vector<DepthInterval>& depthIntervals );

// Short description of the active depth filter for titles/axis labels/group text, e.g.
// "MD 1000 - 2000 m" or "Zones: Valysar, Therys". Empty when filtering is disabled.
QString depthFilterDescription( DepthFilterMode             mode,
                                RiaDefines::DepthType       depthType,
                                double                      depthRangeMin,
                                double                      depthRangeMax,
                                const std::vector<QString>& selectedZones );

// A single observed (e.g. FMU) RFT pressure sample with its uncertainty.
struct ObservedPressure
{
    double  pressure = 0.0;
    double  error    = 0.0; // observed pressure error; 0 if not available
    QString zoneName; // formation zone containing the sample; empty if not filtered by zones
    double  rangeMin = 0.0; // lowest pressure - error among the observations
    double  rangeMax = 0.0; // highest pressure + error among the observations
};

// Observed pressure samples within depthIntervals for the well/time step, across all observed data
// sets. Samples in the same zone are combined into one entry: mean pressure, with rangeMin/rangeMax
// covering all of them. An empty depthIntervals list means no filtering.
std::vector<ObservedPressure> computeObservedPressures( const QString&                    wellName,
                                                        const QDateTime&                  timeStep,
                                                        const std::vector<DepthInterval>& depthIntervals,
                                                        RiaDefines::DepthType             depthType = RiaDefines::DepthType::MEASURED_DEPTH,
                                                        const std::vector<DepthInterval>& zoneIntervals = {} );

// Arithmetic mean of the given samples, or infinity if samples is empty.
double computeMean( const std::vector<double>& samples );

// Computes the individual RFT pressure samples within depthIntervals per ensemble case.
// Indices match ensemble->allSummaryCases(). A case with no data gets an empty vector. An empty
// depthIntervals list means no filtering.
std::vector<std::vector<double>> computePressureSamplesPerCase( RimSummaryEnsemble*               ensemble,
                                                                const QString&                    wellName,
                                                                const QDateTime&                  timeStep,
                                                                RimEclipseResultCase*             eclipseCase,
                                                                const std::vector<DepthInterval>& depthIntervals,
                                                                RiaDefines::DepthType depthType = RiaDefines::DepthType::MEASURED_DEPTH );

// Computes mean RFT pressure per ensemble case (one entry per case, infinity = no data).
// Indices match ensemble->allSummaryCases().
std::vector<double> computeMeanPressurePerCase( RimSummaryEnsemble*               ensemble,
                                                const QString&                    wellName,
                                                const QDateTime&                  timeStep,
                                                RimEclipseResultCase*             eclipseCase,
                                                const std::vector<DepthInterval>& depthIntervals,
                                                RiaDefines::DepthType             depthType = RiaDefines::DepthType::MEASURED_DEPTH );
} // namespace RimRftCrossPlotTools
