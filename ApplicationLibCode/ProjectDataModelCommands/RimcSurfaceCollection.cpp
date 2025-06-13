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
#include "RimcSurfaceCollection.h"

#include "SurfaceCommands/RicImportSurfacesFeature.h"

#include "RimCase.h"
#include "RimFileSurface.h"
#include "RimGridCaseSurface.h"
#include "RimRegularSurface.h"
#include "RimSurface.h"
#include "RimSurfaceCollection.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

#include <QStringList>

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimSurfaceCollection, RimcSurfaceCollection_importSurface, "ImportSurface" );
CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimSurfaceCollection, RimcSurfaceCollection_addFolder, "AddFolder" );
CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimSurfaceCollection, RimcSurfaceCollection_newSurface, "NewSurface" );
CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimSurfaceCollection, RimcSurfaceCollection_newRegularSurface, "NewRegularSurface" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcSurfaceCollection_importSurface::RimcSurfaceCollection_importSurface( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )

{
    CAF_PDM_InitObject( "Import Surface", "", "", "Import a new surface from file" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_fileName, "FileName", "", "", "", "Filename to import surface from" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcSurfaceCollection_importSurface::execute()
{
    RimSurfaceCollection* coll = self<RimSurfaceCollection>();
    if ( coll )
    {
        QStringList filelist;
        filelist << m_fileName();
        return coll->importSurfacesFromFiles( filelist );
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcSurfaceCollection_importSurface::classKeywordReturnedType() const
{
    return RimFileSurface::classKeywordStatic();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcSurfaceCollection_addFolder::RimcSurfaceCollection_addFolder( caf::PdmObjectHandle* self )
    : PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add Folder", "", "", "Add a new surface folder" );

    CAF_PDM_InitScriptableField( &m_folderName, "FolderName", QString( "Surfaces" ), "", "", "", "New surface folder name" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcSurfaceCollection_addFolder::execute()
{
    RimSurfaceCollection* coll = self<RimSurfaceCollection>();
    if ( coll )
    {
        RimSurfaceCollection* newcoll = new RimSurfaceCollection();
        newcoll->setCollectionName( m_folderName() );

        coll->addSubCollection( newcoll );
        return newcoll;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcSurfaceCollection_addFolder::classKeywordReturnedType() const
{
    return RimSurfaceCollection::classKeywordStatic();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcSurfaceCollection_newSurface::RimcSurfaceCollection_newSurface( caf::PdmObjectHandle* self )
    : PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "New Surface", "", "", "Create a new surface" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_case, "Case", "" );
    CAF_PDM_InitScriptableField( &m_kIndex, "KIndex", 0, "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcSurfaceCollection_newSurface::execute()
{
    RimSurfaceCollection* coll = self<RimSurfaceCollection>();
    if ( coll && m_case )
    {
        RimSurface* surface = coll->addGridCaseSurface( m_case(), m_kIndex );
        return surface;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcSurfaceCollection_newSurface::classKeywordReturnedType() const
{
    return RimGridCaseSurface::classKeywordStatic();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcSurfaceCollection_newRegularSurface::RimcSurfaceCollection_newRegularSurface( caf::PdmObjectHandle* self )
    : PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "New Regular Surface", "", "", "Create a new regular surface" );

    CAF_PDM_InitScriptableField( &m_name, "Name", QString( "" ), "Name" );

    CAF_PDM_InitScriptableField( &m_originX, "OriginX", 0.0, "Origin X" );
    CAF_PDM_InitScriptableField( &m_originY, "OriginY", 0.0, "Origin Y" );
    CAF_PDM_InitScriptableField( &m_depth, "Depth", 0.0, "Depth" );

    CAF_PDM_InitScriptableField( &m_nx, "Nx", 10, "Nx" );
    CAF_PDM_InitScriptableField( &m_ny, "Ny", 10, "Ny" );
    CAF_PDM_InitScriptableField( &m_incrementX, "IncrementX", 20.0, "Increment X" );
    CAF_PDM_InitScriptableField( &m_incrementY, "IncrementY", 20.0, "Increment Y" );

    CAF_PDM_InitScriptableField( &m_rotation, "Rotation", 0.0, "Rotation" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcSurfaceCollection_newRegularSurface::execute()
{
    RimSurfaceCollection* coll = self<RimSurfaceCollection>();
    if ( !coll ) return std::unexpected( "No surface collection found" );

    if ( m_nx() <= 0 ) return std::unexpected( "Invalid nx. Must be positive." );
    if ( m_ny() <= 0 ) return std::unexpected( "Invalid ny. Must be positive." );
    if ( m_incrementX() <= 0.0 ) return std::unexpected( "Invalid increment X. Must be positive." );
    if ( m_incrementY() <= 0.0 ) return std::unexpected( "Invalid increment Y. Must be positive." );
    if ( m_rotation() < 0.0 || m_rotation() > 360.0 ) return std::unexpected( "Invalid rotation. Valid range: [0.0-360.0]" );

    RimRegularSurface* surface = new RimRegularSurface;
    surface->setUserDescription( m_name() );

    surface->setNx( m_nx() );
    surface->setNy( m_ny() );
    surface->setOriginX( m_originX() );
    surface->setOriginY( m_originY() );
    surface->setDepth( m_depth() );
    surface->setIncrementX( m_incrementX() );
    surface->setIncrementY( m_incrementY() );
    surface->setRotation( m_rotation() );

    surface->setColor( cvf::Color3f::BLUE );
    surface->setOpacity( true, 0.6f );

    coll->addSurface( surface );
    coll->updateViews();
    coll->updateConnectedEditors();

    return surface;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcSurfaceCollection_newRegularSurface::classKeywordReturnedType() const
{
    return RimRegularSurface::classKeywordStatic();
}
