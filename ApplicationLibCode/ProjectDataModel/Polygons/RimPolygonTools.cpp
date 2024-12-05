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

#include "RimPolygonTools.h"

#include "RiaPreferences.h"

#include "RifCsvDataTableFormatter.h"

#include "RimGridView.h"
#include "RimOilField.h"
#include "RimPolygon.h"
#include "RimPolygonCollection.h"
#include "RimPolygonInView.h"
#include "RimPolygonInViewCollection.h"
#include "RimProject.h"

#include "Riu3DMainWindowTools.h"

#include <QFile>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonTools::activate3dEditOfPolygonInView( RimPolygon* polygon, caf::PdmObject* sourceObject )
{
    auto polygonInView = findPolygonInView( polygon, sourceObject );
    if ( polygonInView )
    {
        polygonInView->enablePicking( true );
        Riu3DMainWindowTools::selectAsCurrentItem( polygonInView );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonTools::selectPolygonInView( RimPolygon* polygon, caf::PdmObject* sourceObject )
{
    auto polygonInView = findPolygonInView( polygon, sourceObject );
    if ( polygonInView )
    {
        Riu3DMainWindowTools::selectAsCurrentItem( polygonInView );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolygonTools::exportPolygonCsv( const RimPolygon* polygon, const QString& filePath )
{
    if ( !polygon ) return false;

    QFile file( filePath );
    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        return false;
    }

    QTextStream out( &file );

    QString                  fieldSeparator = RiaPreferences::current()->csvTextExportFieldSeparator;
    RifCsvDataTableFormatter formatter( out, fieldSeparator );
    const int                precision = 2;

    std::vector<RifTextDataTableColumn> header;
    header.emplace_back( "X", RifTextDataTableDoubleFormatting( RIF_FLOAT, precision ) );
    header.emplace_back( "Y", RifTextDataTableDoubleFormatting( RIF_FLOAT, precision ) );
    header.emplace_back( "Z", RifTextDataTableDoubleFormatting( RIF_FLOAT, precision ) );
    formatter.header( header );

    for ( const auto& point : polygon->pointsInDomainCoords() )
    {
        formatter.add( point.x() );
        formatter.add( point.y() );
        formatter.add( -point.z() );
        formatter.rowCompleted();
    }

    formatter.tableCompleted();

    file.close();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolygonTools::exportPolygonPol( const std::vector<RimPolygon*> polygons, const QString& filePath )
{
    if ( polygons.empty() ) return false;

    QFile file( filePath );
    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) ) return false;

    QTextStream out( &file );

    QString                  fieldSeparator = " ";
    RifCsvDataTableFormatter formatter( out, fieldSeparator );
    const int                precision = 2;

    std::vector<RifTextDataTableColumn> header;
    header.emplace_back( " ", RifTextDataTableDoubleFormatting( RIF_FLOAT, precision ) );
    header.emplace_back( " ", RifTextDataTableDoubleFormatting( RIF_FLOAT, precision ) );
    header.emplace_back( " ", RifTextDataTableDoubleFormatting( RIF_FLOAT, precision ) );
    formatter.header( header );

    for ( auto polygon : polygons )
    {
        for ( const auto& point : polygon->pointsInDomainCoords() )
        {
            formatter.add( point.x() );
            formatter.add( point.y() );
            formatter.add( -point.z() );
            formatter.rowCompleted();
        }

        const double endOfPolygon = 999.0;
        formatter.add( endOfPolygon );
        formatter.add( endOfPolygon );
        formatter.add( endOfPolygon );
        formatter.rowCompleted();
    }

    formatter.tableCompleted();

    file.close();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPolygonTools::polygonCacheName()
{
    return "POLYGON";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonInView* RimPolygonTools::findPolygonInView( RimPolygon* polygon, caf::PdmObject* sourceObject )
{
    if ( !polygon || !sourceObject )
    {
        return nullptr;
    }

    if ( auto gridView = sourceObject->firstAncestorOrThisOfType<RimGridView>() )
    {
        auto polyCollection = gridView->polygonInViewCollection();

        for ( auto polygonInView : polyCollection->allPolygonsInView() )
        {
            if ( polygonInView && polygonInView->polygon() == polygon )
            {
                return polygonInView;
            }
        }
    }

    return nullptr;
}
