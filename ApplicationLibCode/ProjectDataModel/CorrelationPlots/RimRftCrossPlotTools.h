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

#include <QDateTime>
#include <QString>

#include <vector>

class RifReaderRftInterface;
class RigEclipseWellLogExtractor;
class RimEclipseResultCase;
class RimSummaryEnsemble;

//==================================================================================================
///
/// Helpers for filtering and reducing RFT pressure data by depth range, shared by the RFT
/// correlation parameter cross plot (RimParameterRftCrossPlot) and tornado plot (RimRftTornadoPlot).
///
//==================================================================================================
namespace RimRftCrossPlotTools
{
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

// Arithmetic mean of the given samples, or infinity if samples is empty.
double computeMean( const std::vector<double>& samples );

// Computes the individual RFT pressure samples within the depth range per ensemble case.
// Indices match ensemble->allSummaryCases(). A case with no data gets an empty vector.
std::vector<std::vector<double>> computePressureSamplesPerCase( RimSummaryEnsemble*   ensemble,
                                                                const QString&        wellName,
                                                                const QDateTime&      timeStep,
                                                                RimEclipseResultCase* eclipseCase,
                                                                bool                  useDepthRange,
                                                                double                depthRangeMin,
                                                                double                depthRangeMax,
                                                                RiaDefines::DepthType depthType = RiaDefines::DepthType::MEASURED_DEPTH );

// Computes mean RFT pressure per ensemble case (one entry per case, infinity = no data).
// Indices match ensemble->allSummaryCases().
std::vector<double> computeMeanPressurePerCase( RimSummaryEnsemble*   ensemble,
                                                const QString&        wellName,
                                                const QDateTime&      timeStep,
                                                RimEclipseResultCase* eclipseCase,
                                                bool                  useDepthRange,
                                                double                depthRangeMin,
                                                double                depthRangeMax,
                                                RiaDefines::DepthType depthType = RiaDefines::DepthType::MEASURED_DEPTH );
} // namespace RimRftCrossPlotTools
