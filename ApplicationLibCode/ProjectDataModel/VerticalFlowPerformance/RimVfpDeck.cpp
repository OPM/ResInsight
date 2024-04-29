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

#include "RiaOpmParserTools.h"

#include "RimVfpPlotCollection.h"

#include "cafPdmUiTreeOrdering.h"

#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RimVfpDeck, "RimVfpDeck" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpDeck::RimVfpDeck()
{
    CAF_PDM_InitObject( "VFP Plot", ":/VfpPlot.svg" );

    CAF_PDM_InitFieldNoDefault( &m_filePath, "FilePath", "File Path" );
    CAF_PDM_InitFieldNoDefault( &m_vfpPlotCollection, "VfpPlotCollection", "Plot Collection" );
    m_vfpPlotCollection = new RimVfpPlotCollection();

    setDeletable( true );
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
    updateObjectName();

    auto createRimVfpPlot = [&]() -> RimVfpPlot*
    {
        auto plot = new RimVfpPlot();
        plot->setReadDataFromFile( false );
        plot->setFileName( m_filePath().path() );
        return plot;
    };

    std::vector<RimVfpPlot*> currentPlots = m_vfpPlotCollection->plots();

    auto [vfpProdTables, vfpInjTables] = RiaOpmParserTools::extractVfpTablesFromDataFile( m_filePath().path().toStdString() );
    for ( const auto& prodTable : vfpProdTables )
    {
        RimVfpPlot* plot = m_vfpPlotCollection->plotForTableNumber( prodTable.getTableNum() );
        if ( !plot )
        {
            plot = createRimVfpPlot();
            plot->setProductionTable( prodTable );
            m_vfpPlotCollection->addPlot( plot );
        }
        else
        {
            plot->setProductionTable( prodTable );
            std::erase( currentPlots, plot );
        }
        plot->loadDataAndUpdate();
    }

    for ( const auto& injTable : vfpInjTables )
    {
        RimVfpPlot* plot = m_vfpPlotCollection->plotForTableNumber( injTable.getTableNum() );
        if ( !plot )
        {
            plot = createRimVfpPlot();
            plot->setInjectionTable( injTable );
            m_vfpPlotCollection->addPlot( plot );
        }
        else
        {
            plot->setInjectionTable( injTable );
            std::erase( currentPlots, plot );
        }
        plot->loadDataAndUpdate();
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
    QString name = "VFP Plots";

    QFileInfo fileInfo( m_filePath().path() );
    auto      fileName = fileInfo.fileName();
    if ( !fileName.isEmpty() )
    {
        name += " - " + fileName;
    }
    setName( name );
}
