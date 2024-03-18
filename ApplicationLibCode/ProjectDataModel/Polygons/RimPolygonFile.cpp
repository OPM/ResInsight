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

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiTreeAttributes.h"

#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RimPolygonFile, "RimPolygonFileFile" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonFile::RimPolygonFile()
    : objectChanged( this )
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

    updateName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFile::loadData()
{
    auto polygonsFromFile = importDataFromFile( m_fileName().path() );

    if ( m_polygons.size() == polygonsFromFile.size() )
    {
        for ( size_t i = 0; i < m_polygons.size(); i++ )
        {
            auto projectPoly = m_polygons()[i];
            auto filePoly    = polygonsFromFile[i];
            projectPoly->setPointsInDomainCoords( filePoly->pointsInDomainCoords() );
            projectPoly->coordinatesChanged.send(); // updates editors
            projectPoly->objectChanged.send(); // updates filters
            delete filePoly;
        }
    }
    else
    {
        m_polygons.deleteChildren();

        m_polygons.setValue( polygonsFromFile );
    }

    if ( polygonsFromFile.empty() )
    {
        RiaLogging::warning( "No polygons found in file: " + m_fileName().path() );
    }
    else
    {
        RiaLogging::info( QString( "Imported %1 polygons from file: " ).arg( polygonsFromFile.size() ) + m_fileName().path() );
    }
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
QString RimPolygonFile::name() const
{
    QString nameCandidate = RimNamedObject::name();

    if ( !nameCandidate.isEmpty() )
    {
        return nameCandidate;
    }

    auto fileName = m_fileName().path();
    if ( fileName.isEmpty() )
    {
        return "Polygon File";
    }

    QFileInfo fileInfo( fileName );
    return fileInfo.fileName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFile::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( nameField() );
    uiOrdering.add( &m_fileName );
    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFile::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_fileName )
    {
        updateName();

        m_polygons.deleteChildren();
        loadData();
    }

    objectChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygon*> RimPolygonFile::importDataFromFile( const QString& fileName )
{
    QString errorMessages;
    auto    filePolygons = RifPolygonReader::parsePolygonFile( fileName, &errorMessages );

    std::vector<RimPolygon*> polygons;

    for ( const auto& [polygonId, filePolygon] : filePolygons )
    {
        auto polygon = new RimPolygon();
        polygon->disableStorageOfPolygonPoints();
        polygon->setReadOnly( true );

        int id = ( polygonId != -1 ) ? polygonId : static_cast<int>( polygons.size() + 1 );
        polygon->setName( QString( "Polygon %1" ).arg( id ) );
        polygon->setPointsInDomainCoords( filePolygon );
        polygons.push_back( polygon );
    }

    if ( !errorMessages.isEmpty() )
    {
        RiaLogging::error( errorMessages );
    }

    return polygons;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFile::updateName()
{
    QFileInfo fileInfo( m_fileName().path() );
    setName( fileInfo.fileName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFile::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicReloadPolygonFileFeature";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFile::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( m_polygons.empty() )
    {
        caf::PdmUiTreeViewItemAttribute::appendTagToTreeViewItemAttribute( attribute, ":/warning.svg" );
    }
}
