/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RimPressureDepthData.h"

//==================================================================================================
//
//
//
//==================================================================================================
CAF_PDM_SOURCE_INIT( RimPressureDepthData, "ObservedPressureDepthData" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPressureDepthData::RimPressureDepthData()
{
    CAF_PDM_InitObject( "Observed Pressure/Depth Data", ":/ObservedRFTDataFile16x16.png" );
    CAF_PDM_InitFieldNoDefault( &m_filePath, "File", "File" );
    m_filePath.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_wells, "Wells", "Wells" );
    m_wells.xmlCapability()->disableIO();
    m_wells.uiCapability()->setUiReadOnly( true );
    m_wells.registerGetMethod( this, &RimPressureDepthData::wells );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPressureDepthData::setFilePath( const QString& path )
{
    m_filePath = path;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPressureDepthData::createRftReaderInterface()
{
    m_fmuRftReader = new RifReaderPressureDepthData( m_filePath );
    m_fmuRftReader->load();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderRftInterface* RimPressureDepthData::rftReader()
{
    if ( m_fmuRftReader.isNull() )
    {
        createRftReaderInterface();
    }

    return m_fmuRftReader.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPressureDepthData::hasWell( const QString& wellPathName ) const
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
std::vector<QString> RimPressureDepthData::wells() const
{
    if ( m_fmuRftReader.p() )
    {
        std::set<QString> wellNames = const_cast<RifReaderPressureDepthData*>( m_fmuRftReader.p() )->wellNames();
        return std::vector<QString>( wellNames.begin(), wellNames.end() );
    }
    return std::vector<QString>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimPressureDepthData::labels( const RifEclipseRftAddress& rftAddress )
{
    if ( m_fmuRftReader.p() )
    {
        return const_cast<RifReaderPressureDepthData*>( m_fmuRftReader.p() )->labels( rftAddress );
    }
    return {};
}
