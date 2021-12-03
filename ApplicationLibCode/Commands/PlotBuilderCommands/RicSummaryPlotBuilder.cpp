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

#include "RicSummaryPlotBuilder.h"

#include "RimEnsembleCurveSetCollection.h"
#include "RimMainPlotCollection.h"
#include "RimMultiPlot.h"
#include "RimMultiPlotCollection.h"
#include "RimPlot.h"
#include "RimProject.h"
#include "RimSaturationPressurePlot.h"

#include "RifReaderEclipseSummary.h"
#include "RifSummaryReaderInterface.h"

#include "RiaSummaryTools.h"
#include "RifEclipseSummaryAddress.h"
#include "RimEnsembleCurveSet.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlotCollection.h"
#include "RiuPlotMainWindowTools.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSummaryPlotBuilder::RicSummaryPlotBuilder()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotBuilder::setDataSources( const std::vector<RimSummaryCase*>&           summaryCases,
                                            const std::vector<RimSummaryCaseCollection*>& ensembles )
{
    m_summaryCases = summaryCases;
    m_ensembles    = ensembles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotBuilder::setAddresses( const std::set<RifEclipseSummaryAddress>& addresses )
{
    m_addresses = addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotBuilder::setIndividualPlotPerAddress( bool enable )
{
    m_individualPlotPerAddress = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotBuilder::setIndividualPlotPerDataSource( bool enable )
{
    m_individualPlotPerDataSource = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryPlot*> RicSummaryPlotBuilder::createPlots() const
{
    std::vector<RimSummaryPlot*> plots;

    if ( m_individualPlotPerDataSource && m_individualPlotPerAddress )
    {
        for ( auto adr : m_addresses )
        {
            for ( auto summaryCase : m_summaryCases )
            {
                auto plot = createPlot( { adr }, { summaryCase }, {} );
                plots.push_back( plot );
            }

            for ( auto ensemble : m_ensembles )
            {
                auto plot = createPlot( { adr }, {}, { ensemble } );
                plots.push_back( plot );
            }
        }
    }
    else if ( m_individualPlotPerAddress )
    {
        for ( auto adr : m_addresses )
        {
            auto plot = createPlot( { adr }, m_summaryCases, m_ensembles );
            plots.push_back( plot );
        }
    }
    else if ( m_individualPlotPerDataSource )
    {
        for ( auto summaryCase : m_summaryCases )
        {
            auto plot = createPlot( m_addresses, { summaryCase }, {} );
            plots.push_back( plot );
        }

        for ( auto ensemble : m_ensembles )
        {
            auto plot = createPlot( m_addresses, {}, { ensemble } );
            plots.push_back( plot );
        }
    }
    else
    {
        auto plot = createPlot( m_addresses, m_summaryCases, m_ensembles );
        plots.push_back( plot );
    }

    return plots;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RicSummaryPlotBuilder::addressesForSource( caf::PdmObject* summarySource )
{
    auto ensemble = dynamic_cast<RimSummaryCaseCollection*>( summarySource );
    if ( ensemble )
    {
        return ensemble->ensembleSummaryAddresses();
    }

    auto sumCase = dynamic_cast<RimSummaryCase*>( summarySource );
    if ( sumCase )
    {
        auto reader = sumCase ? sumCase->summaryReader() : nullptr;
        if ( reader )
        {
            return reader->allResultAddresses();
        }
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet* RicSummaryPlotBuilder::createCurveSet( RimSummaryCaseCollection*       ensemble,
                                                            const RifEclipseSummaryAddress& addr )
{
    auto curveSet = new RimEnsembleCurveSet();

    curveSet->setSummaryCaseCollection( ensemble );
    curveSet->setSummaryAddress( addr );

    return curveSet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurve* RicSummaryPlotBuilder::createCurve( RimSummaryCase* summaryCase, const RifEclipseSummaryAddress& addr )
{
    auto curve = new RimSummaryCurve();

    curve->setSummaryCaseY( summaryCase );
    curve->setSummaryAddressY( addr );

    return curve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlot*> RicSummaryPlotBuilder::duplicatePlots( const std::vector<RimPlot*>& sourcePlots )
{
    std::vector<RimPlot*> plots;

    for ( auto plot : sourcePlots )
    {
        auto copy = dynamic_cast<RimPlot*>( plot->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );

        {
            // TODO: Workaround for fixing the PdmPointer in RimEclipseResultDefinition
            //    caf::PdmPointer<RimEclipseCase> m_eclipseCase;
            // This pdmpointer must be changed to a ptrField

            auto saturationPressurePlotOriginal = dynamic_cast<RimSaturationPressurePlot*>( plot );
            auto saturationPressurePlotCopy     = dynamic_cast<RimSaturationPressurePlot*>( copy );
            if ( saturationPressurePlotCopy && saturationPressurePlotOriginal )
            {
                RimSaturationPressurePlot::fixPointersAfterCopy( saturationPressurePlotOriginal,
                                                                 saturationPressurePlotCopy );
            }
        }

        plots.push_back( copy );
    }

    return plots;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMultiPlot* RicSummaryPlotBuilder::appendMultiPlot( const std::vector<RimPlot*>& plots )
{
    RimProject*             project        = RimProject::current();
    RimMultiPlotCollection* plotCollection = project->mainPlotCollection()->multiPlotCollection();

    RimMultiPlot* plotWindow = new RimMultiPlot;
    plotWindow->setMultiPlotTitle( QString( "Multi Plot %1" ).arg( plotCollection->multiPlots().size() + 1 ) );
    plotWindow->setAsPlotMdiWindow();
    plotCollection->addMultiPlot( plotWindow );

    for ( auto plot : plots )
    {
        plotWindow->addPlot( plot );

        plot->resolveReferencesRecursively();
        plot->revokeMdiWindowStatus();
        plot->setShowWindow( true );

        plot->loadDataAndUpdate();
    }

    project->updateAllRequiredEditors();
    plotWindow->loadDataAndUpdate();

    RiuPlotMainWindowTools::setExpanded( plotCollection, true );
    RiuPlotMainWindowTools::selectAsCurrentItem( plotWindow, true );

    return plotWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicSummaryPlotBuilder::createPlot( const std::set<RifEclipseSummaryAddress>&     addresses,
                                                   const std::vector<RimSummaryCase*>&           summaryCases,
                                                   const std::vector<RimSummaryCaseCollection*>& ensembles )
{
    RimSummaryPlot* plot = new RimSummaryPlot();
    plot->enableAutoPlotTitle( true );

    appendCurvesToPlot( plot, addresses, summaryCases, ensembles );

    plot->applyDefaultCurveAppearances();

    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotBuilder::appendCurvesToPlot( RimSummaryPlot*                               summaryPlot,
                                                const std::set<RifEclipseSummaryAddress>&     addresses,
                                                const std::vector<RimSummaryCase*>&           summaryCases,
                                                const std::vector<RimSummaryCaseCollection*>& ensembles )
{
    for ( const auto& addr : addresses )
    {
        for ( const auto ensemble : ensembles )
        {
            auto curveSet = createCurveSet( ensemble, addr );
            summaryPlot->ensembleCurveSetCollection()->addCurveSet( curveSet );
        }

        for ( const auto summaryCase : summaryCases )
        {
            auto curve = createCurve( summaryCase, addr );

            summaryPlot->addCurveNoUpdate( curve );
        }
    }
}
