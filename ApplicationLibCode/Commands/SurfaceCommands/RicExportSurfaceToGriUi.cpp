/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RicExportSurfaceToGriUi.h"

#include "cafPdmUiFilePathEditor.h"

#include <cmath>

namespace caf
{
template <>
void AppEnum<RicExportSurfaceToGriUi::ExportFormat>::setUp()
{
    addItem( RicExportSurfaceToGriUi::ExportFormat::GRI, "GRI", "IRAP Binary (.gri)" );
    addItem( RicExportSurfaceToGriUi::ExportFormat::IRAP, "IRAP", "IRAP Classic (.irap)" );
    setDefault( RicExportSurfaceToGriUi::ExportFormat::GRI );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RicExportSurfaceToGriUi, "RicExportSurfaceToGriUi" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicExportSurfaceToGriUi::RicExportSurfaceToGriUi()
{
    CAF_PDM_InitObject( "Export Surface to Regular Grid" );

    CAF_PDM_InitField( &m_exportFormat, "ExportFormat", caf::AppEnum<ExportFormat>( ExportFormat::GRI ), "Format" );

    CAF_PDM_InitFieldNoDefault( &m_exportFolder, "ExportFolder", "Export Folder" );
    m_exportFolder.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_nx, "Nx", 10, "Nx (columns)" );
    CAF_PDM_InitField( &m_ny, "Ny", 10, "Ny (rows)" );
    CAF_PDM_InitField( &m_originX, "OriginX", 0.0, "Origin X" );
    CAF_PDM_InitField( &m_originY, "OriginY", 0.0, "Origin Y" );
    CAF_PDM_InitField( &m_incrementX, "IncrementX", 1.0, "Increment X" );
    CAF_PDM_InitField( &m_incrementY, "IncrementY", 1.0, "Increment Y" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSurfaceToGriUi::setDefaults( const QString& exportFolder, int nx, int ny, double originX, double originY, double incrementX, double incrementY )
{
    m_exportFolder = exportFolder;
    m_nx           = nx;
    m_ny           = ny;
    m_originX      = std::round( originX );
    m_originY      = std::round( originY );
    m_incrementX   = std::round( incrementX );
    m_incrementY   = std::round( incrementY );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigRegularSurfaceData RicExportSurfaceToGriUi::gridParams() const
{
    RigRegularSurfaceData p;
    p.nx         = m_nx;
    p.ny         = m_ny;
    p.originX    = m_originX;
    p.originY    = m_originY;
    p.incrementX = m_incrementX;
    p.incrementY = m_incrementY;
    p.rotation   = 0.0;
    return p;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportSurfaceToGriUi::exportFolder() const
{
    return m_exportFolder;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicExportSurfaceToGriUi::ExportFormat RicExportSurfaceToGriUi::exportFormat() const
{
    return m_exportFormat();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSurfaceToGriUi::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_exportFolder )
    {
        if ( auto* attr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute ) )
        {
            attr->m_selectDirectory = true;
        }
    }
}
