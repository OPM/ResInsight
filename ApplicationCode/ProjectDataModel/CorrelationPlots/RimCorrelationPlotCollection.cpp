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
#include "RimCorrelationReportPlot.h"
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
    CAF_PDM_InitFieldNoDefault( &m_correlationReports, "CorrelationReports", "Correlation Reports", "", "", "" );

    m_correlationPlots.uiCapability()->setUiHidden( true );
    m_correlationReports.uiCapability()->setUiHidden( true );
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
RimCorrelationReportPlot*
    RimCorrelationPlotCollection::createCorrelationReportPlot( bool defaultToFirstEnsembleFopt /*= true */ )
{
    RimCorrelationReportPlot* report = new RimCorrelationReportPlot;
    report->setAsPlotMdiWindow();
    if ( defaultToFirstEnsembleFopt ) applyFirstEnsembleFieldAddressesToReport( report, "FOPT" );

    m_correlationReports.push_back( report );
    return report;
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
std::vector<RimCorrelationReportPlot*> RimCorrelationPlotCollection::reports()
{
    return m_correlationReports.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlotCollection::deleteAllChildObjects()
{
    m_correlationPlots.deleteAllChildObjects();
    m_correlationReports.deleteAllChildObjects();
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlotCollection::applyFirstEnsembleFieldAddressesToReport( RimCorrelationReportPlot* plot,
                                                                             const std::string& quantityName /*= "" */ )
{
    std::vector<RimSummaryCaseCollection*> ensembles;
    RimProject::current()->descendantsIncludingThisOfType( ensembles );
    if ( !ensembles.empty() )
    {
        std::set<RifEclipseSummaryAddress>     allAddresses = ensembles.front()->ensembleSummaryAddresses();
        std::vector<RiaSummaryCurveDefinition> curveDefsMatrix;
        std::vector<RiaSummaryCurveDefinition> curveDefsTornadoAndCrossPlot;
        for ( auto address : allAddresses )
        {
            if ( address.category() == RifEclipseSummaryAddress::SUMMARY_FIELD )
            {
                curveDefsMatrix.push_back( RiaSummaryCurveDefinition( nullptr, address, ensembles.front() ) );
                if ( quantityName.empty() || quantityName == address.quantityName() )
                {
                    curveDefsTornadoAndCrossPlot.push_back(
                        RiaSummaryCurveDefinition( nullptr, address, ensembles.front() ) );
                }
            }
        }
        plot->matrixPlot()->setCurveDefinitions( curveDefsMatrix );
        plot->correlationPlot()->setCurveDefinitions( curveDefsTornadoAndCrossPlot );
        plot->crossPlot()->setCurveDefinitions( curveDefsTornadoAndCrossPlot );
        plot->crossPlot()->setEnsembleParameter( ensembles.front()->alphabeticEnsembleParameters().front().name );
    }
}
