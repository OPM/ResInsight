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
#include "cafPdmUiTreeSelectionEditor.h"

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

    CAF_PDM_InitFieldNoDefault( &m_selectedProperties, "SelectedProperties", "Export Values" );
    m_selectedProperties.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_selectedProperties = std::vector<QString>{ depthEntryName() };

    CAF_PDM_InitField( &m_nx, "Nx", 10, "Nx (columns)" );
    CAF_PDM_InitField( &m_ny, "Ny", 10, "Ny (rows)" );
    CAF_PDM_InitField( &m_originX, "OriginX", 0.0, "Origin X" );
    CAF_PDM_InitField( &m_originY, "OriginY", 0.0, "Origin Y" );
    CAF_PDM_InitField( &m_incrementX, "IncrementX", 1.0, "Increment X" );
    CAF_PDM_InitField( &m_incrementY, "IncrementY", 1.0, "Increment Y" );
    CAF_PDM_InitField( &m_rotation, "Rotation", 0.0, "Rotation" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicExportSurfaceToGriUi::depthEntryName()
{
    return "Depth";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSurfaceToGriUi::setExportFolder( const QString& exportFolder )
{
    m_exportFolder = exportFolder;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSurfaceToGriUi::setGridDefaults( int nx, int ny, double originX, double originY, double incrementX, double incrementY, double rotation )
{
    m_nx         = nx;
    m_ny         = ny;
    m_originX    = originX;
    m_originY    = originY;
    m_incrementX = incrementX;
    m_incrementY = incrementY;
    m_rotation   = rotation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportSurfaceToGriUi::setAvailableProperties( const std::vector<QString>& propertyNames )
{
    m_availableProperties = propertyNames;
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
    p.rotation   = m_rotation;
    return p;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RicExportSurfaceToGriUi::selectedProperties() const
{
    return m_selectedProperties();
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RicExportSurfaceToGriUi::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_selectedProperties )
    {
        options.push_back( caf::PdmOptionItemInfo( depthEntryName(), depthEntryName() ) );

        for ( const auto& name : m_availableProperties )
        {
            // A property with the same name as the depth entry is already represented in the list
            if ( name == depthEntryName() ) continue;

            options.push_back( caf::PdmOptionItemInfo( name, name ) );
        }
    }

    return options;
}
