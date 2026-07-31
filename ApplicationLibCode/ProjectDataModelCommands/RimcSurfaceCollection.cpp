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

#include "RiaNameUniquenessTools.h"

#include "RimCase.h"
#include "RimFileSurface.h"
#include "RimGridCaseSurface.h"
#include "RimRegularSurface.h"
#include "RimSurface.h"
#include "RimSurfaceCollection.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

#include <QFileInfo>
#include <QStringList>

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimSurfaceCollection, RimcSurfaceCollection_importSurface, "ImportSurface" );
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
    CAF_PDM_InitScriptableField( &m_onNameConflict,
                                 "OnNameConflict",
                                 caf::AppEnum<RiaDefines::NameConflictPolicy>( RiaDefines::NameConflictPolicy::FAIL ),
                                 "",
                                 "",
                                 "",
                                 "How to handle a surface name already used in this folder" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcSurfaceCollection_importSurface::execute()
{
    RimSurfaceCollection* coll = self<RimSurfaceCollection>();
    if ( !coll ) return nullptr;

    // Imported surfaces are named after the file, see RimSurfaceCollection::importSurfacesFromFiles
    const QString surfaceName = QFileInfo( m_fileName() ).fileName();

    auto resolution = RiaNameUniquenessTools::applyConflictPolicy( &coll->itemsField(), surfaceName, m_onNameConflict().value() );
    if ( !resolution.errorMessage.isEmpty() ) return std::unexpected( resolution.errorMessage );

    if ( auto* existingSurface = dynamic_cast<RimSurface*>( resolution.objectToReplace ) )
    {
        coll->deleteItem( existingSurface );
    }

    QStringList filelist;
    filelist << m_fileName();
    return coll->importSurfacesFromFiles( filelist );
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
    CAF_PDM_InitScriptableField( &m_onNameConflict,
                                 "OnNameConflict",
                                 caf::AppEnum<RiaDefines::NameConflictPolicy>( RiaDefines::NameConflictPolicy::FAIL ),
                                 "",
                                 "",
                                 "",
                                 "How to handle a surface name already used in this folder" );

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

    auto resolution = RiaNameUniquenessTools::applyConflictPolicy( &coll->itemsField(), m_name(), m_onNameConflict().value() );
    if ( !resolution.errorMessage.isEmpty() ) return std::unexpected( resolution.errorMessage );

    if ( auto* existingSurface = dynamic_cast<RimSurface*>( resolution.objectToReplace ) )
    {
        coll->deleteItem( existingSurface );
    }

    RimRegularSurface* surface = new RimRegularSurface;
    surface->setUserDescription( resolution.nameToUse );

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
