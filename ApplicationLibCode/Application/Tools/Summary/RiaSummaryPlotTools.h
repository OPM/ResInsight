////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

class RimPlot;
class RimMultiPlot;
class RifEclipseSummaryAddress;
class RimSummaryCase;
class RimSummaryEnsemble;
class RimSummaryPlot;
class RimEnsembleCurveSet;
class RimSummaryCurve;
class RimSummaryMultiPlot;
class RiaSummaryCurveAddress;

namespace caf
{
class PdmObject;
class PdmObjectHandle;
} // namespace caf

#include <set>
#include <vector>

//==================================================================================================
///
//==================================================================================================
namespace RiaSummaryPlotTools
{

std::set<RifEclipseSummaryAddress> addressesForSource( caf::PdmObject* summarySource );

RimEnsembleCurveSet* createCurveSet( RimSummaryEnsemble* ensemble, const RifEclipseSummaryAddress& addr );
RimSummaryCurve*     createCurve( RimSummaryCase* summaryCase, const RifEclipseSummaryAddress& addr );

std::vector<RimPlot*>        duplicatePlots( const std::vector<RimPlot*>& plots );
std::vector<RimSummaryPlot*> duplicateSummaryPlots( const std::vector<RimSummaryPlot*>& plots );

RimMultiPlot* createAndAppendMultiPlot( const std::vector<RimPlot*>& plots );
void          appendPlotsToMultiPlot( RimMultiPlot* multiPlot, const std::vector<RimPlot*>& plots );

RimSummaryMultiPlot* createAndAppendDefaultSummaryMultiPlot( const std::vector<RimSummaryCase*>&     cases,
                                                             const std::vector<RimSummaryEnsemble*>& ensembles,
                                                             bool skipCreationOfPlotBasedOnPreferences = true );

RimSummaryMultiPlot* createAndAppendSingleSummaryMultiPlotNoAutoSettings( RimSummaryPlot* plot );
RimSummaryMultiPlot* createAndAppendSingleSummaryMultiPlot( RimSummaryPlot* plot );
RimSummaryMultiPlot* createAndAppendSummaryMultiPlot( const std::vector<RimSummaryPlot*>& plots );
RimSummaryMultiPlot* createAndAppendSummaryMultiPlot( const std::vector<caf::PdmObjectHandle*>& objects );
void                 appendPlotsToSummaryMultiPlot( RimSummaryMultiPlot* multiPlot, const std::vector<RimSummaryPlot*>& plots );

RimSummaryPlot* createPlot( const std::vector<RimSummaryCurve*>& summaryCurves );

RimSummaryPlot* createPlot( const std::set<RifEclipseSummaryAddress>& addresses,
                            const std::vector<RimSummaryCase*>&       summaryCases,
                            const std::vector<RimSummaryEnsemble*>&   ensembles );

RimSummaryPlot* createCrossPlot( const std::vector<RiaSummaryCurveAddress>& addresses,
                                 const std::vector<RimSummaryCase*>&        summaryCases,
                                 const std::vector<RimSummaryEnsemble*>&    ensembles );

void appendCurvesToPlot( RimSummaryPlot*                           summaryPlot,
                         const std::set<RifEclipseSummaryAddress>& addresses,
                         const std::vector<RimSummaryCase*>&       summaryCases,
                         const std::vector<RimSummaryEnsemble*>&   ensembles );

RimEnsembleCurveSet*
    addNewEnsembleCurve( RimSummaryPlot* summaryPlot, const RiaSummaryCurveAddress& curveAddress, RimSummaryEnsemble* ensemble );

RimSummaryCurve* addNewSummaryCurve( RimSummaryPlot* summaryPlot, const RiaSummaryCurveAddress& curveAddress, RimSummaryCase* summaryCase );

}; // namespace RiaSummaryPlotTools
