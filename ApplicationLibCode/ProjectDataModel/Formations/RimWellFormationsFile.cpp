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

#include "RimWellFormationsFile.h"

#include "RiaLogging.h"

#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "Tools/RimCsvPreviewTools.h"

#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RimWellFormationsFile, "WellFormationsFile" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellFormationsFile::RimWellFormationsFile()
{
    CAF_PDM_InitObject( "Well Formations", ":/Formations16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_filePath, "FilePath", "File Path" );
    m_filePath.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );

    CAF_PDM_InitFieldNoDefault( &m_contentTable, "ContentTable", "Content" );
    RimCsvPreviewTools::initPreviewField( m_contentTable );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellFormationsFile::setFilePath( const QString& filePath )
{
    m_filePath = filePath;
    updateUiTreeName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellFormationsFile::filePath() const
{
    return m_filePath().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellFormationsFile::shortName() const
{
    return QFileInfo( m_filePath().path() ).fileName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RimWellFormationsFile::reload()
{
    auto result = RifWellPathFormationReader::readWellFormations( filePath() );
    if ( !result )
    {
        m_wellFormations.clear();
        m_contentTable = "";
        return std::unexpected( result.error() );
    }

    m_wellFormations = std::move( *result );
    m_contentTable   = "";
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimWellFormationsFile::wellNames() const
{
    QStringList names;
    for ( const auto& [wellName, formations] : m_wellFormations )
    {
        names.push_back( wellName );
    }
    return names;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimWellFormationsFile::zoneNames( const QString& wellName ) const
{
    QStringList names;

    auto formations = formationsForWell( wellName );
    if ( !formations ) return names;

    for ( size_t i = 0; i < formations->formationCount(); i++ )
    {
        const QString name = formations->formationAt( i ).formationName;
        if ( !names.contains( name ) ) names.push_back( name );
    }
    return names;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigWellPathFormations* RimWellFormationsFile::formationsForWell( const QString& wellName ) const
{
    if ( auto it = m_wellFormations.find( wellName ); it != m_wellFormations.end() )
    {
        return &it->second;
    }

    // Fall back to a normalized name match, to handle differences between e.g. RFT and FMU well names
    const QString normalizedTarget = normalizedWellName( wellName );
    for ( const auto& [candidateName, formations] : m_wellFormations )
    {
        if ( normalizedWellName( candidateName ) == normalizedTarget )
        {
            return &formations;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// Well names are matched case-insensitively, with '-' and '_' treated as equal
//--------------------------------------------------------------------------------------------------
QString RimWellFormationsFile::normalizedWellName( const QString& wellName )
{
    QString normalized = wellName.trimmed().toLower();
    normalized.replace( '_', '-' );
    return normalized;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellFormationsFile::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_filePath )
    {
        updateUiTreeName();
        if ( auto result = reload(); !result )
        {
            RiaLogging::error( result.error().toStdString() );
        }

        updateReferringObjects( objectsWithReferringPtrFields() );
    }
}

//--------------------------------------------------------------------------------------------------
/// Well paths cache the formations resolved from their file, so they must be refreshed when the
/// file is reloaded or deleted
//--------------------------------------------------------------------------------------------------
void RimWellFormationsFile::updateReferringObjects( const std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    for ( caf::PdmObjectHandle* object : referringObjects )
    {
        if ( auto wellPath = dynamic_cast<RimWellPath*>( object ) )
        {
            wellPath->refreshFormationsFromFile();
            wellPath->updateConnectedEditors();
        }
    }

    RimProject::current()->scheduleCreateDisplayModelAndRedrawAllViews();
    RimMainPlotCollection::current()->updatePlotsWithFormations();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellFormationsFile::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    updateUiTreeName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellFormationsFile::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    // Built on demand, as the table is only needed when the file is shown in the property editor
    if ( m_contentTable().isEmpty() ) updateContentTable();

    uiOrdering.add( &m_filePath );
    uiOrdering.add( &m_contentTable );
    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellFormationsFile::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_contentTable )
    {
        RimCsvPreviewTools::setPreviewEditorAttribute( attribute );
    }
}

//--------------------------------------------------------------------------------------------------
/// Parses the file right after the project is loaded, so the data is available without requiring a
/// manual reload.
//--------------------------------------------------------------------------------------------------
void RimWellFormationsFile::initAfterRead()
{
    if ( m_filePath().path().isEmpty() ) return;

    if ( auto result = reload(); !result )
    {
        RiaLogging::error( result.error().toStdString() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellFormationsFile::updateUiTreeName()
{
    uiCapability()->setUiName( shortName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellFormationsFile::updateContentTable()
{
    m_contentTable = RimCsvPreviewTools::htmlTableFromFile( filePath() );
}
