/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RimVfpDeck.h"

#include "RigVfpTables.h"

#include "RimVfpDataCollection.h"
#include "RimVfpPlotCollection.h"
#include "RimVfpTableData.h"

#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimVfpDeck, "RimVfpDeck" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpDeck::RimVfpDeck()
{
    CAF_PDM_InitObject( "VFP Plot", ":/VfpPlot.svg" );

    CAF_PDM_InitFieldNoDefault( &m_vfpTableData, "VfpTableData", "VFP Data Source" );
    CAF_PDM_InitFieldNoDefault( &m_vfpPlotCollection, "VfpPlotCollection", "Plot Collection" );
    m_vfpPlotCollection = new RimVfpPlotCollection();

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpDeck::setDataSource( RimVfpTableData* tableData )
{
    m_vfpTableData = tableData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpDeck::loadDataAndUpdate()
{
    updateObjectName();

    std::vector<RimVfpPlot*> currentPlots = m_vfpPlotCollection->plots();

    if ( m_vfpTableData )
    {
        m_vfpTableData->ensureDataIsImported();

        if ( m_vfpTableData->vfpTables() )
        {
            auto tables = m_vfpTableData->vfpTables();

            auto allTableNumbers = tables->productionTableNumbers();
            auto injTableNumbers = tables->injectionTableNumbers();
            allTableNumbers.insert( allTableNumbers.end(), injTableNumbers.begin(), injTableNumbers.end() );

            for ( const auto& number : allTableNumbers )
            {
                RimVfpPlot* plot = m_vfpPlotCollection->plotForTableNumber( number );
                if ( !plot )
                {
                    plot = new RimVfpPlot();
                    plot->setDataSource( m_vfpTableData );
                    plot->setTableNumber( number );
                    plot->initializeObject();

                    m_vfpPlotCollection->addPlot( plot );
                }
                else
                {
                    std::erase( currentPlots, plot );
                }
                plot->setDeletable( false );
                plot->loadDataAndUpdate();
            }
        }
    }

    for ( auto plotToDelete : currentPlots )
    {
        m_vfpPlotCollection->removePlot( plotToDelete );
        delete plotToDelete;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimVfpPlot*> RimVfpDeck::plots() const
{
    return m_vfpPlotCollection->plots();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpDeck::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    for ( auto p : m_vfpPlotCollection->plots() )
    {
        uiTreeOrdering.add( p );
    }

    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpDeck::updateObjectName()
{
    QString name = "VFP Deck";

    if ( m_vfpTableData )
    {
        name = m_vfpTableData->name();
    }

    setName( name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimVfpDeck::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_vfpTableData )
    {
        RimVfpDataCollection* vfpDataCollection = RimVfpDataCollection::instance();
        for ( auto table : vfpDataCollection->vfpTableData() )
        {
            options.push_back( caf::PdmOptionItemInfo( table->name(), table ) );
        }
    }

    return options;
}
