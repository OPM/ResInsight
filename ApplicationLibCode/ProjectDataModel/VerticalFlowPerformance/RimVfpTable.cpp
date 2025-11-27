/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RimVfpTable.h"

#include "cafCmdFeatureMenuBuilder.h"

CAF_PDM_SOURCE_INIT( RimVfpTable, "RimVfpTable" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpTable::RimVfpTable()
{
    CAF_PDM_InitObject( "VFP Table", ":/VfpPlot.svg" );

    CAF_PDM_InitFieldNoDefault( &m_dataSource, "DataSource", "Data Source" );
    CAF_PDM_InitFieldNoDefault( &m_tableNumber, "TableNumber", "Table Number" );
    CAF_PDM_InitFieldNoDefault( &m_tableType, "TableType", "Table Type" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpTable::setDataSource( RimVfpTableData* dataSource )
{
    m_dataSource = dataSource;

    updateObjectName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpTable::setTableNumber( int tableNumber )
{
    m_tableNumber = tableNumber;

    updateObjectName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpTable::setTableType( RimVfpDefines::TableType tableType )
{
    m_tableType = tableType;

    updateObjectName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpTable::ensureDataIsImported()
{
    if ( m_dataSource )
    {
        m_dataSource->ensureDataIsImported();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpTableData* RimVfpTable::dataSource() const
{
    return m_dataSource;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimVfpTable::tableNumber() const
{
    return m_tableNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpDefines::TableType RimVfpTable::tableType() const
{
    return m_tableType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpTable::updateObjectName()
{
    if ( m_dataSource )
    {
        QString name;

        if ( m_tableNumber >= 0 )
        {
            if ( !name.isEmpty() ) name += " - ";
            name += QString( "Table %1" ).arg( m_tableNumber() );
        }

        if ( m_tableType() == RimVfpDefines::TableType::INJECTION )
        {
            if ( !name.isEmpty() ) name += " - ";
            name += QString( "INJ" );
        }
        else
        {
            if ( !name.isEmpty() ) name += " - ";
            name += QString( "PROD" );
        }

        if ( !name.isEmpty() ) name += " - ";
        name += m_dataSource->name();

        setName( name );
    }
    else
    {
        setName( "VFP Table" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpTable::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicNewCustomVfpPlotFeature";
}
