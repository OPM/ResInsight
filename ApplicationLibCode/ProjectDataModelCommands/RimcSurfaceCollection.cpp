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
#include "RimSurface.h"
#include "RimSurfaceCollection.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

#include <QStringList>

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimSurfaceCollection, RimcSurfaceCollection_importSurface, "ImportSurface" );
CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimSurfaceCollection, RimcSurfaceCollection_addFolder, "AddFolder" );
CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimSurfaceCollection, RimcSurfaceCollection_newSurface, "NewSurface" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcSurfaceCollection_importSurface::RimcSurfaceCollection_importSurface( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Import Surface", "", "", "Import a new surface from file" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_fileName, "FileName", "", "", "", "Filename to import surface from" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimcSurfaceCollection_importSurface::execute()
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
bool RimcSurfaceCollection_importSurface::resultIsPersistent() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcSurfaceCollection_importSurface::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimFileSurface );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcSurfaceCollection_importSurface::isNullptrValidResult() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcSurfaceCollection_addFolder::RimcSurfaceCollection_addFolder( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Add Folder", "", "", "Add a new surface folder" );
    CAF_PDM_InitScriptableField( &m_folderName, "FolderName", QString( "Surfaces" ), "", "", "", "New surface folder name" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimcSurfaceCollection_addFolder::execute()
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
bool RimcSurfaceCollection_addFolder::resultIsPersistent() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcSurfaceCollection_addFolder::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimSurfaceCollection );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcSurfaceCollection_addFolder::isNullptrValidResult() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcSurfaceCollection_newSurface::RimcSurfaceCollection_newSurface( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "New Surface", "", "", "Create a new surface" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_case, "Case", "", "", "", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_kIndex, "KIndex", "", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObjectHandle* RimcSurfaceCollection_newSurface::execute()
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
bool RimcSurfaceCollection_newSurface::resultIsPersistent() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcSurfaceCollection_newSurface::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimGridCaseSurface );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcSurfaceCollection_newSurface::isNullptrValidResult() const
{
    return true;
}
