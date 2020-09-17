/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016  Statoil ASA
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

#include "RimSummaryPlotCollection.h"

#include "RiaGuiApplication.h"
#include "RiuPlotMainWindow.h"

#include "RimProject.h"
#include "RimSummaryPlot.h"

#include "cafPdmFieldReorderCapability.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmObjectMethod.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimSummaryPlotCollection, "SummaryPlotCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlotCollection::RimSummaryPlotCollection()
{
    CAF_PDM_InitScriptableObject( "Summary Plots", ":/SummaryPlotsLight16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_summaryPlots, "SummaryPlots", "Summary Plots", "", "", "" );
    m_summaryPlots.uiCapability()->setUiHidden( true );
    caf::PdmFieldReorderCapability::addToField( &m_summaryPlots );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlotCollection::~RimSummaryPlotCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RimSummaryPlotCollection::createSummaryPlotWithAutoTitle()
{
    RimSummaryPlot* plot = new RimSummaryPlot();
    plot->setAsPlotMdiWindow();

    plot->enableAutoPlotTitle( true );

    addPlot( plot );

    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RimSummaryPlotCollection::createNamedSummaryPlot( const QString& name )
{
    RimSummaryPlot* plot = new RimSummaryPlot();
    plot->setAsPlotMdiWindow();

    addPlot( plot );

    plot->setDescription( name );

    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotCollection::updateSummaryNameHasChanged()
{
    for ( RimSummaryPlot* plot : plots() )
    {
        plot->updateCaseNameHasChanged();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotCollection::summaryPlotItemInfos( QList<caf::PdmOptionItemInfo>* optionInfos ) const
{
    for ( RimSummaryPlot* plot : plots() )
    {
        QString displayName = plot->description();
        optionInfos->push_back( caf::PdmOptionItemInfo( displayName, plot, false, plot->uiCapability()->uiIconProvider() ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotCollection::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                               std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    updateSummaryNameHasChanged();
    RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
    mainPlotWindow->updateSummaryPlotToolBar();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryPlot*> RimSummaryPlotCollection::plots() const
{
    return m_summaryPlots.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimSummaryPlotCollection::plotCount() const
{
    return m_summaryPlots.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotCollection::insertPlot( RimSummaryPlot* summaryPlot, size_t index )
{
    m_summaryPlots.insert( index, summaryPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotCollection::removePlot( RimSummaryPlot* summaryPlot )
{
    m_summaryPlots.removeChildObject( summaryPlot );
    updateAllRequiredEditors();
}
