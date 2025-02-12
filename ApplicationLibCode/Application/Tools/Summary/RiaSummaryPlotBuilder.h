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

class RifEclipseSummaryAddress;
class RimSummaryCase;
class RimSummaryEnsemble;
class RimSummaryPlot;

#include <set>
#include <vector>

//==================================================================================================
///
//==================================================================================================
class RiaSummaryPlotBuilder
{
public:
    enum class RicGraphCurveGrouping
    {
        SINGLE_CURVES,
        CURVES_FOR_OBJECT,
        NONE
    };

public:
    RiaSummaryPlotBuilder();

    void setDataSources( const std::vector<RimSummaryCase*>& summaryCases, const std::vector<RimSummaryEnsemble*>& ensembles );

    void setAddresses( const std::set<RifEclipseSummaryAddress>& addresses );

    void setIndividualPlotPerDataSource( bool enable );
    void setGrouping( RicGraphCurveGrouping groping );

    std::vector<RimSummaryPlot*> createPlots() const;

private:
    std::set<RifEclipseSummaryAddress> m_addresses;
    std::vector<RimSummaryCase*>       m_summaryCases;
    std::vector<RimSummaryEnsemble*>   m_ensembles;

    bool m_individualPlotPerDataSource;

    RicGraphCurveGrouping m_graphCurveGrouping;
};
