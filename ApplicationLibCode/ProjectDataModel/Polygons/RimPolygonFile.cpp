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

#include "RimPolygonFile.h"

#include "RiaLogging.h"

#include "RifPolygonReader.h"

#include "RimPolygon.h"

#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RimPolygonFile, "RimPolygonFileFile" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonFile::RimPolygonFile()
{
    CAF_PDM_InitObject( "PolygonFile", ":/PolylinesFromFile16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_fileName, "StimPlanFileName", "File Name" );
    CAF_PDM_InitFieldNoDefault( &m_polygons, "Polygons", "Polygons" );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFile::setFileName( const QString& fileName )
{
    m_fileName = fileName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFile::loadData()
{
    loadPolygonsFromFile();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygon*> RimPolygonFile::polygons() const
{
    return m_polygons.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFile::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFile::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    loadPolygonsFromFile();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFile::loadPolygonsFromFile()
{
    m_polygons.deleteChildren();

    QFileInfo fi( m_fileName().path() );

    QFile dataFile( m_fileName().path() );

    if ( !dataFile.open( QFile::ReadOnly ) )
    {
        RiaLogging::error( "Could not open the File: " + ( m_fileName().path() ) + "\n" );
        return;
    }

    QTextStream stream( &dataFile );
    auto        fileContent = stream.readAll();

    QString errorMessages;

    if ( fi.suffix().trimmed().toLower() == "csv" )
    {
        auto filePolygons = RifPolygonReader::parseTextCsv( fileContent, &errorMessages );

        for ( const auto& [polygonId, filePolygon] : filePolygons )
        {
            auto polygon = new RimPolygon();

            int id = ( polygonId != -1 ) ? polygonId : static_cast<int>( m_polygons.size() + 1 );
            polygon->setName( QString( "Polygon %1" ).arg( id ) );
            polygon->setPointsInDomainCoords( filePolygon );
            m_polygons.push_back( polygon );
        }
    }
    else
    {
        auto filePolygons = RifPolygonReader::parseText( fileContent, &errorMessages );

        for ( const auto& filePolygon : filePolygons )
        {
            auto polygon = new RimPolygon();

            int id = static_cast<int>( m_polygons.size() + 1 );
            polygon->setName( QString( "Polygon %1" ).arg( id ) );
            polygon->setPointsInDomainCoords( filePolygon );
            m_polygons.push_back( polygon );
        }
    }

    if ( !errorMessages.isEmpty() )
    {
        RiaLogging::error( errorMessages );
    }
}
