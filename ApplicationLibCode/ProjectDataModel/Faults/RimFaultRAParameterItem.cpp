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

#include "RimFaultRAParameterItem.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiLineEditor.h"

#include <cmath>

CAF_PDM_SOURCE_INIT( RimFaultRAParameterItem, "FaultRAParameterItem" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultRAParameterItem::RimFaultRAParameterItem()
{
    CAF_PDM_InitField( &m_parameterName, "Name", QString(), "Name", "", "", "" );
    m_parameterName.uiCapability()->setUiReadOnly( true );
    CAF_PDM_InitField( &m_parameterValue, "Value", QString(), "Value", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFaultRAParameterItem::~RimFaultRAParameterItem()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultRAParameterItem::setValue( QString parametername, QString parametervalue )
{
    m_parameterName  = parametername;
    m_parameterValue = parametervalue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFaultRAParameterItem::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                const QVariant&            oldValue,
                                                const QVariant&            newValue )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultRAParameterItem::isString() const
{
    return std::isnan( doubleValue() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFaultRAParameterItem::isNumber() const
{
    return !isString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRAParameterItem::stringValue() const
{
    return m_parameterValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFaultRAParameterItem::doubleValue() const
{
    bool   bOk    = true;
    double retval = m_parameterValue().toDouble( &bOk );
    if ( !bOk ) retval = std::nan( "" );

    return retval;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFaultRAParameterItem::parameterName() const
{
    return m_parameterName;
}
