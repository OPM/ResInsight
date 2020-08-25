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
    CAF_PDM_InitObject( "Ensemble Correlation Plots", ":/CorrelationPlots16x16.png", "", "" );

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

    if ( defaultToFirstEnsembleFopt ) applyFirstEnsembleFieldAddressesToPlot( plot, {"FOPT"} );
    plot->selectAllParameters();

    m_correlationPlots.push_back( plot );

    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationPlot* RimCorrelationPlotCollection::createCorrelationPlot( RimSummaryCaseCollection* ensemble,
                                                                         const QString&            quantityName,
                                                                         std::time_t               timeStep )
{
    RimCorrelationPlot* plot = new RimCorrelationPlot();
    plot->setAsPlotMdiWindow();

    applyEnsembleFieldAndTimeStepToPlot( plot, ensemble, {quantityName}, timeStep );
    plot->selectAllParameters();

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
    plot->selectAllParameters();

    m_correlationPlots.push_back( plot );

    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationMatrixPlot* RimCorrelationPlotCollection::createCorrelationMatrixPlot( RimSummaryCaseCollection* ensemble,
                                                                                     const std::vector<QString>& quantityNames,
                                                                                     std::time_t timeStep )
{
    RimCorrelationMatrixPlot* plot = new RimCorrelationMatrixPlot();
    plot->setAsPlotMdiWindow();
    applyEnsembleFieldAndTimeStepToPlot( plot, ensemble, quantityNames, timeStep );
    plot->selectAllParameters();

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
    if ( defaultToFirstEnsembleFopt ) applyFirstEnsembleFieldAddressesToPlot( plot, {"FOPT"} );

    m_correlationPlots.push_back( plot );
    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimParameterResultCrossPlot* RimCorrelationPlotCollection::createParameterResultCrossPlot( RimSummaryCaseCollection* ensemble,
                                                                                           const QString& paramName,
                                                                                           const QString& quantityName,
                                                                                           std::time_t    timeStep )
{
    RimParameterResultCrossPlot* plot = new RimParameterResultCrossPlot;
    plot->setAsPlotMdiWindow();
    applyEnsembleFieldAndTimeStepToPlot( plot, ensemble, {quantityName}, timeStep );
    plot->setEnsembleParameter( paramName );

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
    if ( defaultToFirstEnsembleFopt ) applyFirstEnsembleFieldAddressesToReport( report, {"FOPT"}, "FOPT" );
    report->matrixPlot()->selectAllParameters();
    report->correlationPlot()->selectAllParameters();
    m_correlationReports.push_back( report );
    return report;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCorrelationReportPlot*
    RimCorrelationPlotCollection::createCorrelationReportPlot( RimSummaryCaseCollection*   ensemble,
                                                               const std::vector<QString>& matrixQuantityNames,
                                                               const QString& tornadoAndCrossPlotQuantityName,
                                                               std::time_t    timeStep )
{
    RimCorrelationReportPlot* report = new RimCorrelationReportPlot;
    report->setAsPlotMdiWindow();
    applyEnsembleFieldAndTimeStepToReport( report, ensemble, matrixQuantityNames, tornadoAndCrossPlotQuantityName, timeStep );
    report->matrixPlot()->selectAllParameters();
    report->correlationPlot()->selectAllParameters();
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
                                                                           const std::vector<QString>& quantityNames /*= {} */ )
{
    std::vector<RimSummaryCaseCollection*> ensembles;
    RimProject::current()->descendantsIncludingThisOfType( ensembles );
    if ( !ensembles.empty() )
    {
        std::set<RifEclipseSummaryAddress>     allAddresses = ensembles.front()->ensembleSummaryAddresses();
        std::vector<RiaSummaryCurveDefinition> curveDefs;
        for ( auto address : allAddresses )
        {
            auto it = std::find( quantityNames.begin(), quantityNames.end(), QString::fromStdString( address.uiText() ) );
            if ( it != quantityNames.end() || quantityNames.empty() )
            {
                curveDefs.push_back( RiaSummaryCurveDefinition( nullptr, address, ensembles.front() ) );
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
void RimCorrelationPlotCollection::applyEnsembleFieldAndTimeStepToPlot( RimAbstractCorrelationPlot* plot,
                                                                        RimSummaryCaseCollection*   ensemble,
                                                                        const std::vector<QString>& quantityNames,
                                                                        std::time_t                 timeStep )
{
    if ( ensemble )
    {
        std::set<RifEclipseSummaryAddress>     allAddresses = ensemble->ensembleSummaryAddresses();
        std::vector<RiaSummaryCurveDefinition> curveDefs;
        for ( auto address : allAddresses )
        {
            auto it = std::find( quantityNames.begin(), quantityNames.end(), QString::fromStdString( address.uiText() ) );
            if ( it != quantityNames.end() || quantityNames.empty() )
            {
                curveDefs.push_back( RiaSummaryCurveDefinition( nullptr, address, ensemble ) );
            }
        }
        plot->setCurveDefinitions( curveDefs );
        plot->setTimeStep( timeStep );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlotCollection::applyFirstEnsembleFieldAddressesToReport( RimCorrelationReportPlot* plot,
                                                                             const std::vector<QString>& matrixQuantityNames,
                                                                             const QString& tornadoAndCrossPlotQuantityName )
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
            auto it = std::find( matrixQuantityNames.begin(),
                                 matrixQuantityNames.end(),
                                 QString::fromStdString( address.uiText() ) );
            if ( it != matrixQuantityNames.end() || matrixQuantityNames.empty() )
            {
                curveDefsMatrix.push_back( RiaSummaryCurveDefinition( nullptr, address, ensembles.front() ) );
            }

            if ( tornadoAndCrossPlotQuantityName.isEmpty() ||
                 tornadoAndCrossPlotQuantityName == QString::fromStdString( address.uiText() ) )
            {
                curveDefsTornadoAndCrossPlot.push_back( RiaSummaryCurveDefinition( nullptr, address, ensembles.front() ) );
            }
        }

        plot->matrixPlot()->setCurveDefinitions( curveDefsMatrix );
        plot->correlationPlot()->setCurveDefinitions( curveDefsTornadoAndCrossPlot );
        plot->crossPlot()->setCurveDefinitions( curveDefsTornadoAndCrossPlot );
        plot->crossPlot()->setEnsembleParameter( ensembles.front()->variationSortedEnsembleParameters().front().name );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimCorrelationPlotCollection::applyEnsembleFieldAndTimeStepToReport( RimCorrelationReportPlot*   plot,
                                                                          RimSummaryCaseCollection*   ensemble,
                                                                          const std::vector<QString>& matrixQuantityNames,
                                                                          const QString& tornadoAndCrossPlotQuantityName,
                                                                          std::time_t    timeStep )
{
    if ( ensemble )
    {
        std::set<RifEclipseSummaryAddress>     allAddresses = ensemble->ensembleSummaryAddresses();
        std::vector<RiaSummaryCurveDefinition> curveDefsMatrix;
        std::vector<RiaSummaryCurveDefinition> curveDefsTornadoAndCrossPlot;
        for ( auto address : allAddresses )
        {
            auto it = std::find( matrixQuantityNames.begin(),
                                 matrixQuantityNames.end(),
                                 QString::fromStdString( address.uiText() ) );
            if ( it != matrixQuantityNames.end() || matrixQuantityNames.empty() )
            {
                curveDefsMatrix.push_back( RiaSummaryCurveDefinition( nullptr, address, ensemble ) );
            }

            if ( tornadoAndCrossPlotQuantityName.isEmpty() ||
                 tornadoAndCrossPlotQuantityName == QString::fromStdString( address.uiText() ) )
            {
                curveDefsTornadoAndCrossPlot.push_back( RiaSummaryCurveDefinition( nullptr, address, ensemble ) );
            }
        }
        plot->matrixPlot()->setCurveDefinitions( curveDefsMatrix );
        plot->matrixPlot()->setTimeStep( timeStep );
        plot->correlationPlot()->setCurveDefinitions( curveDefsTornadoAndCrossPlot );
        plot->correlationPlot()->setTimeStep( timeStep );
        plot->crossPlot()->setCurveDefinitions( curveDefsTornadoAndCrossPlot );
        plot->crossPlot()->setTimeStep( timeStep );
        plot->crossPlot()->setEnsembleParameter( ensemble->variationSortedEnsembleParameters().front().name );
    }
}
