/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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

#include <QString>

#include <set>
#include <vector>

class RimSummaryCase;
class RimPlotCurve;
class RigEnsembleParameter;
class RimSummaryEnsemble;
class RimDeltaSummaryEnsemble;

namespace RimSummaryEnsembleTools
{
std::set<QString> wellsWithRftData( const std::vector<RimSummaryCase*>& summaryCases );

RigEnsembleParameter              createEnsembleParameter( const std::vector<RimSummaryCase*>& summaryCases, const QString& paramName );
void                              sortByBinnedVariation( std::vector<RigEnsembleParameter>& parameterVector );
std::vector<RigEnsembleParameter> alphabeticEnsembleParameters( const std::vector<RimSummaryCase*>& summaryCases );
std::vector<RigEnsembleParameter> createVariationSortedEnsembleParameters( const std::vector<RimSummaryCase*>& summaryCases );

size_t calculateEnsembleParametersIntersectionHash( const std::vector<RimSummaryCase*>& summaryCases );

bool                         isEnsembleCurve( RimPlotCurve* sourceCurve );
std::vector<RimSummaryCase*> summaryCasesFromCurves( const std::vector<RimPlotCurve*>& sourceCurves );
void                         selectSummaryCasesInProjectTree( const std::vector<RimSummaryCase*>& sourceCases );
void                         highlightCurvesForSummaryCases( const std::vector<RimSummaryCase*>& sourceCases );
void                         resetHighlightAllPlots();

RimSummaryCase* caseWithMostDataObjects( const std::vector<RimSummaryCase*>& sourceCases );

// The delta ensembles referring to sourceEnsemble, one entry per delta ensemble. An ensemble used as both source 1 and
// source 2 is reported once.
std::vector<RimDeltaSummaryEnsemble*> dependentDeltaEnsembles( const RimSummaryEnsemble* sourceEnsemble );

// All delta ensembles depending directly or indirectly on any of sourceEnsembles, ordered so that a delta ensemble
// always appears before the delta ensembles using it as a source. Cycles are reported and not traversed.
std::vector<RimDeltaSummaryEnsemble*> deltaEnsemblesInUpdateOrder( const std::vector<RimSummaryEnsemble*>& sourceEnsembles );

// True if using candidateSource as a source for deltaEnsemble would make the dependency graph cyclic.
bool wouldCreateDependencyCycle( const RimDeltaSummaryEnsemble* deltaEnsemble, const RimSummaryEnsemble* candidateSource );

void updateDependentDeltaEnsembles( const RimSummaryEnsemble* sourceEnsemble );

} // namespace RimSummaryEnsembleTools
