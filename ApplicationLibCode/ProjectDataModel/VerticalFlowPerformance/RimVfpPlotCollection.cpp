/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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

#include "RimVfpPlotCollection.h"

#include "RiaApplication.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigEquil.h"

#include "RimEclipseResultCase.h"
#include "RimProject.h"
#include "RimVfpDeck.h"

#include "cafCmdFeatureMenuBuilder.h"

CAF_PDM_SOURCE_INIT( RimVfpPlotCollection, "RimVfpPlotCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpPlotCollection::RimVfpPlotCollection()
{
    CAF_PDM_InitObject( "VFP Plots", ":/VfpPlotCollection.svg" );

    CAF_PDM_InitFieldNoDefault( &m_vfpPlots, "VfpPlots", "Vertical Flow Performance Plots" );
    CAF_PDM_InitFieldNoDefault( &m_vfpDecks, "VfpDecks", "Vertical Flow Performance Decks" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpPlotCollection::~RimVfpPlotCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlotCollection::addPlot( RimVfpPlot* newPlot )
{
    m_vfpPlots.push_back( newPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlotCollection::insertPlot( RimVfpPlot* vfpPlot, size_t index )
{
    m_vfpPlots.insert( index, vfpPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimVfpPlot*> RimVfpPlotCollection::plots() const
{
    return m_vfpPlots.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlotCollection::deleteChildren()
{
    m_vfpPlots.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpPlot* RimVfpPlotCollection::plotForTableNumber( int tableNumber ) const
{
    for ( auto plot : plots() )
    {
        if ( plot->tableNumber() == tableNumber )
        {
            return plot;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimVfpPlotCollection::plotCount() const
{
    return m_vfpPlots.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlotCollection::removePlot( RimVfpPlot* vfpPlot )
{
    m_vfpPlots.removeChild( vfpPlot );
    updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpDeck* RimVfpPlotCollection::addDeck( const QString& filename )
{
    RimVfpDeck* deck = new RimVfpDeck();
    deck->setFileName( filename );
    m_vfpDecks.push_back( deck );

    return deck;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlotCollection::onChildrenUpdated( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& updatedObjects )
{
    for ( auto plot : plots() )
    {
        plot->updateMdiWindowVisibility();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlotCollection::loadDataAndUpdateAllPlots()
{
    for ( auto plot : plots() )
    {
        plot->loadDataAndUpdate();
    }

    for ( auto deck : m_vfpDecks.childrenByType() )
    {
        deck->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlotCollection::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    // A variant with a true value is used to indicate that the VFP data is imported from a file
    // This is used to distinguish between VFP data imported from a file and VFP data imported from a simulator
    QVariant variant( QVariant::fromValue( true ) );
    menuBuilder.addCmdFeatureWithUserData( "RicImportVfpDataFeature", "Import VFP Files", variant );
    menuBuilder.addCmdFeature( "RicImportVfpDataFeature", "Import VFP from Simulator Files" );
}
