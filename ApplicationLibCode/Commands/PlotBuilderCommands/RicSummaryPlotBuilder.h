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
class RimSummaryCaseCollection;
class RimSummaryPlot;
class RimEnsembleCurveSet;
class RimSummaryCurve;
class RimSummaryMultiPlot;

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
class RicSummaryPlotBuilder
{
public:
    enum class RicGraphCurveGrouping
    {
        SINGLE_CURVES,
        CURVES_FOR_OBJECT,
        NONE
    };

public:
    RicSummaryPlotBuilder();

    void setDataSources( const std::vector<RimSummaryCase*>&           summaryCases,
                         const std::vector<RimSummaryCaseCollection*>& ensembles );

    void setAddresses( const std::set<RifEclipseSummaryAddress>& addresses );

    void setIndividualPlotPerDataSource( bool enable );
    void setGrouping( RicGraphCurveGrouping groping );

    std::vector<RimSummaryPlot*> createPlots() const;

    // Static helper functions
    static std::set<RifEclipseSummaryAddress> addressesForSource( caf::PdmObject* summarySource );

    static RimEnsembleCurveSet* createCurveSet( RimSummaryCaseCollection* ensemble, const RifEclipseSummaryAddress& addr );
    static RimSummaryCurve*     createCurve( RimSummaryCase* summaryCase, const RifEclipseSummaryAddress& addr );

    static std::vector<RimPlot*>        duplicatePlots( const std::vector<RimPlot*>& plots );
    static std::vector<RimSummaryPlot*> duplicateSummaryPlots( const std::vector<RimSummaryPlot*>& plots );

    static RimMultiPlot* createAndAppendMultiPlot( const std::vector<RimPlot*>& plots );
    static void          appendPlotsToMultiPlot( RimMultiPlot* multiPlot, const std::vector<RimPlot*>& plots );

    static RimSummaryMultiPlot*
        createAndAppendDefaultSummaryMultiPlot( const std::vector<RimSummaryCase*>&           cases,
                                                const std::vector<RimSummaryCaseCollection*>& ensembles );

    static RimSummaryMultiPlot* createAndAppendSummaryMultiPlot( const std::vector<RimSummaryPlot*>& plots );
    static RimSummaryMultiPlot* createAndAppendSummaryMultiPlot( const std::vector<caf::PdmObjectHandle*>& objects );
    static void appendPlotsToSummaryMultiPlot( RimSummaryMultiPlot* multiPlot, const std::vector<RimSummaryPlot*>& plots );

    static RimSummaryPlot* createPlot( const std::set<RifEclipseSummaryAddress>&     addresses,
                                       const std::vector<RimSummaryCase*>&           summaryCases,
                                       const std::vector<RimSummaryCaseCollection*>& ensembles );

    static void appendCurvesToPlot( RimSummaryPlot*                               summaryPlot,
                                    const std::set<RifEclipseSummaryAddress>&     addresses,
                                    const std::vector<RimSummaryCase*>&           summaryCases,
                                    const std::vector<RimSummaryCaseCollection*>& ensembles );

private:
    std::set<RifEclipseSummaryAddress>     m_addresses;
    std::vector<RimSummaryCase*>           m_summaryCases;
    std::vector<RimSummaryCaseCollection*> m_ensembles;

    bool m_individualPlotPerDataSource;

    RicGraphCurveGrouping m_graphCurveGrouping;
};
