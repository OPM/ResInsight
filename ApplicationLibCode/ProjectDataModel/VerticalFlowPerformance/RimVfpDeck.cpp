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

#include "RimVfpDeck.h"

#include "RiaOpmParserTools.h"

#include "RimVfpPlotCollection.h"

CAF_PDM_SOURCE_INIT( RimVfpDeck, "RimVfpDeck" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpDeck::RimVfpDeck()
{
    // TODO: add icon
    CAF_PDM_InitObject( "VFP Plot", ":/VfpPlot.svg" );

    CAF_PDM_InitFieldNoDefault( &m_filePath, "FilePath", "File Path" );
    CAF_PDM_InitFieldNoDefault( &m_vfpPlotCollection, "VfpPlotCollection", "Plot Collection" );
    m_vfpPlotCollection = new RimVfpPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpDeck::setFileName( const QString& filename )
{
    m_filePath = filename;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpDeck::loadDataAndUpdate()
{
    m_vfpPlotCollection->deleteAllPlots();

    auto [vfpProdTables, vfpInjTables] = RiaOpmParserTools::extractVfpTablesFromDataFile( m_filePath().path().toStdString() );
    for ( auto prodTable : vfpProdTables )
    {
        auto plot = new RimVfpPlot();
        plot->setVfpTable( prodTable );
    }
}
