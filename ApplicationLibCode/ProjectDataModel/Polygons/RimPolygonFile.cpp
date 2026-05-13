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
#include "RiaQStringFormatter.h"

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
    CAF_PDM_InitObject( "PolygonFile", ":/Folder.png" );

    // Inherited fields. The polygon items keep the original "Polygons" keyword so
    // existing files load. The previous Name field was provided by RimNamedObject under
    // keyword "Name"; reuse that keyword for m_collectionName for backward compatibility.
    CAF_PDM_InitFieldNoDefault( &m_collectionName, "Name", "Name" );

    CAF_PDM_InitFieldNoDefault( &m_subCollections, "SubCollections", "Subcollections" );
    m_subCollections.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_items, "Polygons", "Polygons" );

    CAF_PDM_InitFieldNoDefault( &m_fileName, "FileName", "File Name" );
    m_fileName.registerKeywordAlias( "StimPlanFileName" );
    m_fileName.uiCapability()->setUiReadOnly( true );

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

    if ( polygonsFromFile.size() == 1 )
    {
        polygonsFromFile[0]->setName( name() );
    }

    auto existingPolygons = items();
    if ( existingPolygons.size() == polygonsFromFile.size() )
    {
        for ( size_t i = 0; i < existingPolygons.size(); i++ )
        {
            auto projectPoly = existingPolygons[i];
            projectPoly->setDeletable( false );
            auto filePoly = polygonsFromFile[i];
            projectPoly->setPointsInDomainCoords( filePoly->pointsInDomainCoords() );
            projectPoly->coordinatesChanged.send(); // updates editors
            projectPoly->objectChanged.send(); // updates filters
            delete filePoly;
        }
    }
    else
    {
        m_items.deleteChildren();

        m_items.setValue( polygonsFromFile );
    }

    if ( polygonsFromFile.empty() )
    {
        RiaLogging::warning( "No polygons found in file: " + m_fileName().path().toStdString() );
    }
    else
    {
        RiaLogging::info( std::format( "Imported {} polygon(s) from file: {}", polygonsFromFile.size(), m_fileName().path() ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygon*> RimPolygonFile::polygons() const
{
    return items();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPolygonFile::name() const
{
    QString nameCandidate = m_collectionName.value();

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
bool RimPolygonFile::canAddSubCollection() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonContainer* RimPolygonFile::addNewSubCollection()
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFile::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_collectionName );
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

        m_items.deleteChildren();
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

    QFileInfo     fi( fileName );
    const QString basename = fi.baseName();

    for ( const auto& [polygonId, filePolygon] : filePolygons )
    {
        auto polygon = new RimPolygon();
        polygon->disableStorageOfPolygonPoints();
        polygon->setReadOnly( true );
        polygon->setDeletable( false );

        int id = ( polygonId != -1 ) ? polygonId : static_cast<int>( polygons.size() + 1 );
        polygon->setName( QString( "%1 (%2)" ).arg( basename ).arg( id ) );
        polygon->setPointsInDomainCoords( filePolygon );
        polygons.push_back( polygon );
    }

    if ( !errorMessages.isEmpty() )
    {
        RiaLogging::error( errorMessages.toStdString() );
    }

    return polygons;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFile::updateName()
{
    QFileInfo fileInfo( m_fileName().path() );
    setCollectionName( fileInfo.baseName() );
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
    if ( m_items.empty() )
    {
        caf::PdmUiTreeViewItemAttribute::appendTagToTreeViewItemAttribute( attribute, ":/warning.svg" );
    }
}
