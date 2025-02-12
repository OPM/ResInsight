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

#include "RiaSummaryPlotBuilder.h"

#include "Summary/RiaSummaryAddressAnalyzer.h"
#include "Summary/RiaSummaryPlotTools.h"
#include "Summary/RiaSummaryTools.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryPlotBuilder::RiaSummaryPlotBuilder()
    : m_individualPlotPerDataSource( false )
    , m_graphCurveGrouping( RiaSummaryPlotBuilder::RicGraphCurveGrouping::NONE )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryPlotBuilder::setDataSources( const std::vector<RimSummaryCase*>& summaryCases, const std::vector<RimSummaryEnsemble*>& ensembles )
{
    m_summaryCases = summaryCases;
    m_ensembles    = ensembles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryPlotBuilder::setAddresses( const std::set<RifEclipseSummaryAddress>& addresses )
{
    m_addresses = addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryPlotBuilder::setIndividualPlotPerDataSource( bool enable )
{
    m_individualPlotPerDataSource = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSummaryPlotBuilder::setGrouping( RicGraphCurveGrouping groping )
{
    m_graphCurveGrouping = groping;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryPlot*> RiaSummaryPlotBuilder::createPlots() const
{
    std::vector<RimSummaryPlot*> plots;

    if ( m_individualPlotPerDataSource )
    {
        if ( m_graphCurveGrouping == RicGraphCurveGrouping::SINGLE_CURVES )
        {
            for ( const auto& adr : m_addresses )
            {
                for ( auto summaryCase : m_summaryCases )
                {
                    auto plot = RiaSummaryPlotTools::createPlot( { adr }, { summaryCase }, {} );
                    plots.push_back( plot );
                }

                for ( auto ensemble : m_ensembles )
                {
                    auto plot = RiaSummaryPlotTools::createPlot( { adr }, {}, { ensemble } );
                    plots.push_back( plot );
                }
            }
        }
        else if ( m_graphCurveGrouping == RicGraphCurveGrouping::CURVES_FOR_OBJECT )
        {
            RiaSummaryAddressAnalyzer analyzer;
            analyzer.appendAddresses( m_addresses );

            auto groups = analyzer.addressesGroupedByObject();
            for ( const auto& group : groups )
            {
                std::set<RifEclipseSummaryAddress> addresses;
                addresses.insert( group.begin(), group.end() );
                if ( addresses.empty() ) continue;

                for ( auto summaryCase : m_summaryCases )
                {
                    auto plot = RiaSummaryPlotTools::createPlot( addresses, { summaryCase }, {} );
                    plots.push_back( plot );
                }

                for ( auto ensemble : m_ensembles )
                {
                    auto plot = RiaSummaryPlotTools::createPlot( addresses, {}, { ensemble } );
                    plots.push_back( plot );
                }
            }
        }
        else if ( m_graphCurveGrouping == RicGraphCurveGrouping::NONE )
        {
            for ( auto summaryCase : m_summaryCases )
            {
                auto plot = RiaSummaryPlotTools::createPlot( m_addresses, { summaryCase }, {} );
                plots.push_back( plot );
            }

            for ( auto ensemble : m_ensembles )
            {
                auto plot = RiaSummaryPlotTools::createPlot( m_addresses, {}, { ensemble } );
                plots.push_back( plot );
            }
        }
    }
    else // all data sources in same plot
    {
        if ( m_graphCurveGrouping == RicGraphCurveGrouping::SINGLE_CURVES )
        {
            for ( const auto& adr : m_addresses )
            {
                if ( !m_summaryCases.empty() )
                {
                    auto plot = RiaSummaryPlotTools::createPlot( { adr }, m_summaryCases, {} );
                    plots.push_back( plot );
                }

                if ( !m_ensembles.empty() )
                {
                    auto plot = RiaSummaryPlotTools::createPlot( { adr }, {}, m_ensembles );
                    plots.push_back( plot );
                }
            }
        }
        else if ( m_graphCurveGrouping == RicGraphCurveGrouping::CURVES_FOR_OBJECT )
        {
            RiaSummaryAddressAnalyzer analyzer;
            analyzer.appendAddresses( m_addresses );

            auto groups = analyzer.addressesGroupedByObject();
            for ( const auto& group : groups )
            {
                std::set<RifEclipseSummaryAddress> addresses;
                addresses.insert( group.begin(), group.end() );
                if ( addresses.empty() ) continue;

                if ( !m_summaryCases.empty() )
                {
                    auto plot = RiaSummaryPlotTools::createPlot( addresses, m_summaryCases, {} );
                    plots.push_back( plot );
                }

                if ( !m_ensembles.empty() )
                {
                    auto plot = RiaSummaryPlotTools::createPlot( addresses, {}, m_ensembles );
                    plots.push_back( plot );
                }
            }
        }
        else if ( m_graphCurveGrouping == RicGraphCurveGrouping::NONE )
        {
            if ( !m_summaryCases.empty() )
            {
                auto plot = RiaSummaryPlotTools::createPlot( m_addresses, m_summaryCases, {} );
                plots.push_back( plot );
            }

            if ( !m_ensembles.empty() )
            {
                auto plot = RiaSummaryPlotTools::createPlot( m_addresses, {}, m_ensembles );
                plots.push_back( plot );
            }
        }
    }

    return plots;
}
