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

#include "RimCustomVfpPlot.h"
#include "RimVfpTable.h"

#include "cafCmdFeatureMenuBuilder.h"

CAF_PDM_SOURCE_INIT( RimVfpPlotCollection, "RimVfpPlotCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpPlotCollection::RimVfpPlotCollection()
{
    CAF_PDM_InitObject( "VFP Plots", ":/VfpPlotCollection.svg" );

    CAF_PDM_InitFieldNoDefault( &m_customVfpPlots, "CustomVfpPlots", "Vertical Flow Performance Plots" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCustomVfpPlot* RimVfpPlotCollection::createAndAppendPlots( RimVfpTable* mainDataSource, std::vector<RimVfpTable*> tableData )
{
    auto vfpPlot = new RimCustomVfpPlot();
    vfpPlot->selectDataSource( mainDataSource, tableData );
    vfpPlot->initializeObject();
    vfpPlot->initializeSelection();
    vfpPlot->createDefaultColors();

    m_customVfpPlots.push_back( vfpPlot );

    vfpPlot->loadDataAndUpdate();

    return vfpPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlotCollection::addPlot( RimCustomVfpPlot* newPlot )
{
    m_customVfpPlots.push_back( newPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlotCollection::insertPlot( RimCustomVfpPlot* vfpPlot, size_t index )
{
    m_customVfpPlots.insert( index, vfpPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimCustomVfpPlot*> RimVfpPlotCollection::plots() const
{
    return m_customVfpPlots.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimVfpPlotCollection::plotCount() const
{
    return m_customVfpPlots.size() + m_customVfpPlots.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlotCollection::removePlot( RimCustomVfpPlot* vfpPlot )
{
    m_customVfpPlots.removeChild( vfpPlot );
    updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlotCollection::deleteAllPlots()
{
    m_customVfpPlots.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlotCollection::addImportItems( caf::CmdFeatureMenuBuilder& menuBuilder )
{
    // A variant with a true value is used to indicate that the VFP data is imported from a file
    // This is used to distinguish between VFP data imported from a file and VFP data imported from a simulator
    QVariant variant( QVariant::fromValue( true ) );
    menuBuilder.addCmdFeatureWithUserData( "RicImportVfpDataFeature", "Import VFP Files", variant );
    menuBuilder.addCmdFeature( "RicImportVfpDataFeature", "Import VFP from Simulator Files" );
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

    for ( auto customPlot : m_customVfpPlots.childrenByType() )
    {
        customPlot->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpPlotCollection::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    addImportItems( menuBuilder );
}
