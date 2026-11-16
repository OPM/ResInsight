/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025- Equinor ASA
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

#include "RimcPolygonCollection.h"

#include "RiaNameUniquenessTools.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonCollection.h"
#include "Polygons/RimPolygonFile.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapabilityCvfVec3d.h"

#include <QFileInfo>

#include <memory>

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimPolygonCollection, RimcPolygonCollection_createPolygon, "CreatePolygon" );
CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimPolygonCollection, RimcPolygonCollection_importPolygonFile, "ImportPolygonFile" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcPolygonCollection_createPolygon::RimcPolygonCollection_createPolygon( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Create and Add New Polygon", "", "", "Create and Add New Polygon" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_name, "Name", "Name" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_coordinates, "Coordinates", "Coordinates" );
    CAF_PDM_InitScriptableField( &m_onNameConflict,
                                 "OnNameConflict",
                                 caf::AppEnum<RiaDefines::NameConflictPolicy>( RiaDefines::NameConflictPolicy::FAIL ),
                                 "",
                                 "",
                                 "",
                                 "How to handle a polygon name already used in this folder" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcPolygonCollection_createPolygon::execute()
{
    auto polygonCollection = self<RimPolygonCollection>();

    QString                 name   = m_name();
    std::vector<cvf::Vec3d> coords = m_coordinates();

    if ( !name.isEmpty() )
    {
        auto resolution = RiaNameUniquenessTools::applyConflictPolicy( &polygonCollection->itemsField(), name, m_onNameConflict().value() );
        if ( !resolution.errorMessage.isEmpty() ) return std::unexpected( resolution.errorMessage );

        // The new polygon is added below, and that refreshes the views for both objects
        if ( auto* existingPolygon = dynamic_cast<RimPolygon*>( resolution.objectToReplace ) )
        {
            polygonCollection->deleteItem( existingPolygon );
        }

        name = resolution.nameToUse;
    }

    RimPolygon* polygon = new RimPolygon();
    if ( !name.isEmpty() )
    {
        polygon->setName( name );
    }

    polygon->setPointsInDomainCoords( coords );

    polygonCollection->addUserDefinedPolygon( polygon );
    polygonCollection->updateConnectedEditors();

    return polygon;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcPolygonCollection_createPolygon::classKeywordReturnedType() const
{
    return RimPolygon::classKeywordStatic();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcPolygonCollection_importPolygonFile::RimcPolygonCollection_importPolygonFile( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Import Polygon File",
                        "",
                        "",
                        "Import polygons from file into a new folder named after the file. Returns the first polygon in the file" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_fileName, "FileName", "", "", "", "Polygon file (*.pol, *.dat, *.txt, *.csv)" );
    CAF_PDM_InitScriptableField( &m_onNameConflict,
                                 "OnNameConflict",
                                 caf::AppEnum<RiaDefines::NameConflictPolicy>( RiaDefines::NameConflictPolicy::FAIL ),
                                 "",
                                 "",
                                 "",
                                 "How to handle a folder name already used in this folder" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcPolygonCollection_importPolygonFile::execute()
{
    auto polygonCollection = self<RimPolygonCollection>();
    if ( !polygonCollection ) return std::unexpected( "Polygon collection is null. Cannot import polygon file." );

    const QString fileName = m_fileName();
    if ( fileName.isEmpty() ) return std::unexpected( "File name is empty. Cannot import polygon file." );

    QFileInfo fileInfo( fileName );
    if ( !fileInfo.exists() ) return std::unexpected( QString( "Polygon file does not exist: %1" ).arg( fileName ) );

    // The folder is named after the file, see RimPolygonFile::updateName
    auto* subCollections = polygonCollection->subCollectionsField();
    auto  resolution     = RiaNameUniquenessTools::applyConflictPolicy( subCollections, fileInfo.baseName(), m_onNameConflict().value() );
    if ( !resolution.errorMessage.isEmpty() ) return std::unexpected( resolution.errorMessage );

    auto polygonFile = std::make_unique<RimPolygonFile>();
    polygonFile->setFileName( fileName );
    polygonFile->setCollectionName( resolution.nameToUse );

    // Load before deleting a folder to overwrite, so a failed import keeps the existing folder
    polygonFile->loadData();
    if ( polygonFile->polygons().empty() ) return std::unexpected( QString( "No polygons found in file: %1" ).arg( fileName ) );

    if ( resolution.objectToReplace )
    {
        size_t index = subCollections->indexOf( resolution.objectToReplace );
        if ( index < subCollections->size() )
        {
            subCollections->erase( index );
            delete resolution.objectToReplace;
        }
    }

    auto* polygonFilePtr = polygonFile.release();
    polygonCollection->addPolygonFile( polygonFilePtr );
    polygonCollection->updateConnectedEditors();

    return polygonFilePtr->polygons().front();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcPolygonCollection_importPolygonFile::classKeywordReturnedType() const
{
    return RimPolygon::classKeywordStatic();
}
