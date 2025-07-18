/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021- Equinor ASA
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

#include "RimcSurface.h"

#include "RimCase.h"
#include "RimSurface.h"

#include "Surface/RigSurface.h"

#include "RifSurfaceExporter.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

#include <QStringList>

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimSurface, RimcSurface_exportToFile, "ExportToFile" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcSurface_exportToFile::RimcSurface_exportToFile( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Export Surface To file", "", "", "Export a surface to file" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_fileName, "FileName", "", "", "", "Filename to export surface to" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcSurface_exportToFile::execute()
{
    RimSurface* surface = self<RimSurface>();
    if ( !surface )
    {
        return std::unexpected( "No surface found" );
    }

    RigSurface* surfaceData = surface->surfaceData();
    if ( !surfaceData )
    {
        return std::unexpected( "No surface data found" );
    }

    if ( !RifSurfaceExporter::writeGocadTSurfFile( m_fileName(),
                                                   surface->userDescription(),
                                                   surfaceData->vertices(),
                                                   surfaceData->triangleIndices() ) )
    {
        return std::unexpected( QString( "Failed to write surface to file: '%1'" ).arg( m_fileName() ) );
    }

    return nullptr;
}
