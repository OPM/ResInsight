/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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
#include "RimObservedFmuRftData.h"

#include "Tools/RimCsvPreviewTools.h"

//==================================================================================================
//
//
//
//==================================================================================================
CAF_PDM_SOURCE_INIT( RimObservedFmuRftData, "ObservedFmuRftData" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimObservedFmuRftData::RimObservedFmuRftData()
{
    CAF_PDM_InitObject( "Observed FMU Data", ":/ObservedRFTDataFile16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_directoryPath, "ObservedFolder", "Directory" );
    m_directoryPath.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_directoryPath_OBSOLETE, "Directory", "Directory" );
    m_directoryPath_OBSOLETE.uiCapability()->setUiReadOnly( true );
    m_directoryPath_OBSOLETE.xmlCapability()->setIOWritable( false );

    CAF_PDM_InitFieldNoDefault( &m_contentTable, "ContentTable", "Content" );
    RimCsvPreviewTools::initPreviewField( m_contentTable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimObservedFmuRftData::setDirectoryPath( const QString& path )
{
    m_directoryPath = path;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimObservedFmuRftData::createRftReaderInterface()
{
    m_fmuRftReader = std::make_unique<RifReaderFmuRft>( m_directoryPath().path() );
    m_fmuRftReader->importData();
    m_contentTable = "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderRftInterface* RimObservedFmuRftData::rftReader()
{
    if ( !m_fmuRftReader )
    {
        createRftReaderInterface();
    }

    return m_fmuRftReader.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimObservedFmuRftData::hasWell( const QString& wellPathName ) const
{
    std::vector<QString> allWells = wells();
    for ( const QString& well : allWells )
    {
        if ( well == wellPathName )
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimObservedFmuRftData::wells() const
{
    if ( m_fmuRftReader )
    {
        std::set<QString> wellNames = m_fmuRftReader->wellNames();
        return std::vector<QString>( wellNames.begin(), wellNames.end() );
    }
    return std::vector<QString>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimObservedFmuRftData::labels( const RifEclipseRftAddress& rftAddress )
{
    if ( m_fmuRftReader )
    {
        return m_fmuRftReader->labels( rftAddress );
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimObservedFmuRftData::initAfterRead()
{
    if ( m_directoryPath().path().isEmpty() )
    {
        m_directoryPath = m_directoryPath_OBSOLETE();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimObservedFmuRftData::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    // Built on demand, as the table is only needed when the data is shown in the property editor
    if ( m_contentTable().isEmpty() ) updateContentTable();

    uiOrdering.add( nameField() );
    uiOrdering.add( &m_directoryPath );
    uiOrdering.add( &m_contentTable );
    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimObservedFmuRftData::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_contentTable )
    {
        RimCsvPreviewTools::setPreviewEditorAttribute( attribute );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimObservedFmuRftData::updateContentTable()
{
    if ( !rftReader() ) return;

    m_contentTable = RimCsvPreviewTools::htmlTableFromText( m_fmuRftReader->csvText() );
}
