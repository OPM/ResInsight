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
#include "RimSummaryCase.h"
#include "RimSummaryPlot.h"

#include "cafPdmFieldReorderCapability.h"

CAF_PDM_SOURCE_INIT( RimSummaryTableCollection, "RimSummaryTableCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryTableCollection::RimSummaryTableCollection()
{
    CAF_PDM_InitObject( "Summary Tables", ":/CorrelationMatrixPlot16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_items, "SummaryTables", "Summary Tables" );
    caf::PdmFieldReorderCapability::addToField( &m_items );
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
    deleteAllItems();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTableCollection::loadDataAndUpdateAllPlots()
{
    for ( RimSummaryTable* table : items() )
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
    return count();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryTable*> RimSummaryTableCollection::tables() const
{
    return items();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTableCollection::addTable( RimSummaryTable* table )
{
    addItem( table );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryTableCollection::removeTable( RimSummaryTable* table )
{
    deleteItem( table );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryTable* RimSummaryTableCollection::createDefaultSummaryTable()
{
    RimSummaryTable* table = new RimSummaryTable();
    table->setDefaultCaseAndCategoryAndVectorName();
    table->setAsPlotMdiWindow();

    return table;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryTable* RimSummaryTableCollection::createSummaryTableFromCategoryAndVectorName( RimSummaryCase* summaryCase,
                                                                                         RifEclipseSummaryAddressDefines::SummaryCategory category,
                                                                                         const QString& vectorName )
{
    RimSummaryTable* table = new RimSummaryTable();
    table->setFromCaseAndCategoryAndVectorName( summaryCase, category, vectorName );
    table->setAsPlotMdiWindow();

    return table;
}
