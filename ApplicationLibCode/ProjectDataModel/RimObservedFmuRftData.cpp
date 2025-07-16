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

    CAF_PDM_InitFieldNoDefault( &m_wells, "Wells", "Wells" );
    m_wells.xmlCapability()->disableIO();
    m_wells.uiCapability()->setUiReadOnly( true );
    m_wells.registerGetMethod( this, &RimObservedFmuRftData::wells );
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
