/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 -    Equinor ASA
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

#include "RimPressureTableItem.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiLineEditor.h"

CAF_PDM_SOURCE_INIT( RimPressureTableItem, "PressureTableItem" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPressureTableItem::RimPressureTableItem()
    : changed( this )
{
    CAF_PDM_InitField( &m_depth, "Depth", 0.0, "Depth TVDMSL [m]", "", "", "" );
    CAF_PDM_InitField( &m_initialPressure, "InitialPressure", 0.0, "Initial Pressure [Bar]", "", "", "" );
    CAF_PDM_InitField( &m_pressure, "Pressure", 0.0, "Pressure [Bar]", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPressureTableItem::~RimPressureTableItem()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPressureTableItem::setValues( double depth, double initialPressure, double pressure )
{
    m_depth           = depth;
    m_initialPressure = initialPressure;
    m_pressure        = pressure;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimPressureTableItem::depth() const
{
    return m_depth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimPressureTableItem::initialPressure() const
{
    return m_initialPressure;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimPressureTableItem::pressure() const
{
    return m_pressure;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPressureTableItem::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                             const QVariant&            oldValue,
                                             const QVariant&            newValue )
{
    changed.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPressureTableItem::setPressureDate( const QString& pressureDate )
{
    if ( pressureDate.isEmpty() )
        m_pressure.uiCapability()->setUiName( "Pressure [Bar]" );
    else
        m_pressure.uiCapability()->setUiName( QString( "Pressure %1 [Bar]" ).arg( pressureDate ) );
}
