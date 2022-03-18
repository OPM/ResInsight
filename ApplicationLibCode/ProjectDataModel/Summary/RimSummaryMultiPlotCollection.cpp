/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022 Equinor ASA
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
#include "RimSummaryMultiPlotCollection.h"

#include "RimProject.h"
#include "RimSummaryMultiPlot.h"

#include "RiuPlotMainWindowTools.h"

#include "cafPdmFieldReorderCapability.h"

CAF_PDM_SOURCE_INIT( RimSummaryMultiPlotCollection, "RimSummaryMultiPlotCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlotCollection::RimSummaryMultiPlotCollection()
{
    CAF_PDM_InitObject( "Summary Multi Plots", ":/MultiPlot16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_summaryMultiPlots, "MultiSummaryPlots", "Multi Summary Plots" );
    m_summaryMultiPlots.uiCapability()->setUiTreeHidden( true );
    caf::PdmFieldReorderCapability::addToField( &m_summaryMultiPlots );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlotCollection::~RimSummaryMultiPlotCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlotCollection::initAfterRead()
{
    for ( auto& plot : m_summaryMultiPlots )
    {
        plot->duplicatePlot.connect( this, &RimSummaryMultiPlotCollection::onDuplicatePlot );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlotCollection::deleteAllPlots()
{
    m_summaryMultiPlots.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryMultiPlot*> RimSummaryMultiPlotCollection::multiPlots() const
{
    return m_summaryMultiPlots.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlotCollection::addSummaryMultiPlot( RimSummaryMultiPlot* plot )
{
    m_summaryMultiPlots().push_back( plot );
    plot->duplicatePlot.connect( this, &RimSummaryMultiPlotCollection::onDuplicatePlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlotCollection::loadDataAndUpdateAllPlots()
{
    for ( const auto& p : m_summaryMultiPlots.childObjects() )
        p->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimSummaryMultiPlotCollection::plotCount() const
{
    return m_summaryMultiPlots.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryMultiPlotCollection::onDuplicatePlot( const caf::SignalEmitter* emitter, RimSummaryMultiPlot* plotToDuplicate )
{
    if ( !plotToDuplicate ) return;

    auto plotCopy = dynamic_cast<RimSummaryMultiPlot*>(
        plotToDuplicate->copyByXmlSerialization( caf::PdmDefaultObjectFactory::instance() ) );

    addSummaryMultiPlot( plotCopy );

    plotCopy->resolveReferencesRecursively();
    plotCopy->initAfterReadRecursively();
    plotCopy->updateAllRequiredEditors();
    plotCopy->loadDataAndUpdate();

    updateConnectedEditors();

    RiuPlotMainWindowTools::selectAsCurrentItem( plotCopy, true );
}
