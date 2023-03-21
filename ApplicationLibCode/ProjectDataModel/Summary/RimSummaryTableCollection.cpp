/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017  Statoil ASA
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

#include "RimSummaryTableCollection.h"

#include "RimProject.h"
#include "RimSummaryCrossPlot.h"
#include "RimSummaryPlot.h"

#include "cafPdmFieldReorderCapability.h"

CAF_PDM_SOURCE_INIT( RimSummaryTableCollection, "RimSummaryTableCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryTableCollection::RimSummaryTableCollection()
{
    CAF_PDM_InitObject( "Summary Tables", ":/MultiPlot16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_summaryTables, "SummaryTables", "Summary Tables" );
    m_summaryTables.uiCapability()->setUiTreeHidden( true );
    caf::PdmFieldReorderCapability::addToField( &m_summaryTables );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryTableCollection::~RimSummaryTableCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTableCollection::deleteAllPlots()
{
    m_summaryTables.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTableCollection::loadDataAndUpdateAllPlots()
{
    for ( RimSummaryTable* table : m_summaryTables )
    {
        if ( !table ) continue;

        table->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimSummaryTableCollection::plotCount() const
{
    return tableCount();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryTable*> RimSummaryTableCollection::tables() const
{
    return m_summaryTables.children();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimSummaryTableCollection::tableCount() const
{
    return m_summaryTables.size();
}

void RimSummaryTableCollection::addTable( RimSummaryTable* table )
{
    insertTable( table, tableCount() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTableCollection::updateSummaryNameHasChanged()
{
    // TODO: UPDATE??
    for ( RimSummaryTable* table : m_summaryTables )
    {
        // plot->updateCaseNameHasChanged();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTableCollection::insertTable( RimSummaryTable* table, size_t index )
{
    m_summaryTables.insert( index, table );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTableCollection::removeTable( RimSummaryTable* table )
{
    m_summaryTables.removeChild( table );
    updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryTable* RimSummaryTableCollection::createSummaryTable()
{
    RimSummaryTable* table = new RimSummaryTable();
    table->setAsPlotMdiWindow();
    table->setDescription( QString( "Summary Table %1" ).arg( m_summaryTables.size() ) );

    return table;
}
