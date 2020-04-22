/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-  Equinor ASA
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

#include "RimCorrelationPlotCollection.h"

#include "RiaSummaryCurveDefinition.h"
#include "RimCorrelationMatrixPlot.h"
#include "RimCorrelationPlot.h"
#include "RimParameterResultCrossPlot.h"
#include "RimProject.h"

CAF_PDM_SOURCE_INIT( RimCorrelationPlotCollection, "CorrelationPlotCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationPlotCollection::RimCorrelationPlotCollection()
{
    CAF_PDM_InitObject( "Correlation Plots", ":/AnalysisPlots16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_correlationPlots, "CorrelationPlots", "Correlation Plots", "", "", "" );
    m_correlationPlots.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationPlotCollection::~RimCorrelationPlotCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationPlot* RimCorrelationPlotCollection::createCorrelationPlot( bool defaultToFirstEnsembleFopt )
{
    RimCorrelationPlot* plot = new RimCorrelationPlot();
    plot->setAsPlotMdiWindow();

    if ( defaultToFirstEnsembleFopt ) applyFirstEnsembleFieldAddressesToPlot( plot, "FOPT" );

    m_correlationPlots.push_back( plot );

    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationMatrixPlot* RimCorrelationPlotCollection::createCorrelationMatrixPlot( bool defaultToFirstEnsembleField )
{
    RimCorrelationMatrixPlot* plot = new RimCorrelationMatrixPlot();
    plot->setAsPlotMdiWindow();
    if ( defaultToFirstEnsembleField ) applyFirstEnsembleFieldAddressesToPlot( plot );

    m_correlationPlots.push_back( plot );

    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimParameterResultCrossPlot* RimCorrelationPlotCollection::createParameterResultCrossPlot( bool defaultToFirstEnsembleFopt )
{
    RimParameterResultCrossPlot* plot = new RimParameterResultCrossPlot;
    plot->setAsPlotMdiWindow();
    if ( defaultToFirstEnsembleFopt ) applyFirstEnsembleFieldAddressesToPlot( plot, "FOPT" );

    m_correlationPlots.push_back( plot );
    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlotCollection::removePlot( RimAbstractCorrelationPlot* correlationPlot )
{
    m_correlationPlots.removeChildObject( correlationPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimAbstractCorrelationPlot*> RimCorrelationPlotCollection::plots()
{
    return m_correlationPlots.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlotCollection::deleteAllChildObjects()
{
    m_correlationPlots.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlotCollection::applyFirstEnsembleFieldAddressesToPlot( RimAbstractCorrelationPlot* plot,
                                                                           const std::string& quantityName /*= "FOPT" */ )
{
    std::vector<RimSummaryCaseCollection*> ensembles;
    RimProject::current()->descendantsIncludingThisOfType( ensembles );
    if ( !ensembles.empty() )
    {
        std::set<RifEclipseSummaryAddress>     allAddresses = ensembles.front()->ensembleSummaryAddresses();
        std::vector<RiaSummaryCurveDefinition> curveDefs;
        for ( auto address : allAddresses )
        {
            if ( address.category() == RifEclipseSummaryAddress::SUMMARY_FIELD )
            {
                if ( quantityName.empty() || quantityName == address.quantityName() )
                {
                    curveDefs.push_back( RiaSummaryCurveDefinition( nullptr, address, ensembles.front() ) );
                }
            }
        }
        plot->setCurveDefinitions( curveDefs );

        auto crossPlot = dynamic_cast<RimParameterResultCrossPlot*>( plot );
        if ( crossPlot )
        {
            crossPlot->setEnsembleParameter( ensembles.front()->alphabeticEnsembleParameters().front().name );
        }
    }
}
