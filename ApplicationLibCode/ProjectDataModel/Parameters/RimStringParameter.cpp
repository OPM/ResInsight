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

#include "RimStringParameter.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiLineEditor.h"

#include <cmath>

CAF_PDM_SOURCE_INIT( RimStringParameter, "StringParameter" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStringParameter::RimStringParameter()
{
    CAF_PDM_InitField( &m_value, "Value", QString(), "Value", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStringParameter::~RimStringParameter()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStringParameter::setValue( QString value )
{
    m_value = value;
    setValid( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant RimStringParameter::variantValue() const
{
    return QVariant( m_value() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStringParameter::stringValue() const
{
    return m_value();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStringParameter::jsonValue() const
{
    return QString( "\"%1\"" ).arg( stringValue() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGenericParameter* RimStringParameter::duplicate() const
{
    RimStringParameter* retval = new RimStringParameter();
    retval->setName( name() );
    retval->setValue( stringValue() );
    retval->setDescription( description() );
    retval->setLabel( label() );
    retval->setAdvanced( isAdvanced() );

    return retval;
}
