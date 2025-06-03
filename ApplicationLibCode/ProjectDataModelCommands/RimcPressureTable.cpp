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
#include "RimcPressureTable.h"

#include "RimPressureTable.h"
#include "RimPressureTableItem.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimPressureTable, RimcPressureTable_addPressure, "AddPressure" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcPressureTable_addPressure::RimcPressureTable_addPressure( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Add pressure data", "", "", "Add pressure data to pressure table." );
    setNullptrValid( false );
    setResultPersistent( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_depth, "Depth", "", "", "", "Depth: TVDMSL [m]" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_initialPressure, "InitialPressure", "", "", "", "Initial Pressure [Bar]" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_pressure, "Pressure", "", "", "", "Pressure [Bar]" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcPressureTable_addPressure::execute()
{
    RimPressureTableItem* pressureTableItem = new RimPressureTableItem;
    pressureTableItem->setValues( m_depth, m_initialPressure, m_pressure );

    RimPressureTable* pressureTable = self<RimPressureTable>();
    pressureTable->insertItem( nullptr, pressureTableItem );

    pressureTable->updateAllRequiredEditors();
    return pressureTableItem;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcPressureTable_addPressure::resultIsPersistent_obsolete() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcPressureTable_addPressure::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimPressureTableItem );
}
