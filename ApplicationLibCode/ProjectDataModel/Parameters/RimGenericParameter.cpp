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

#include "RimGenericParameter.h"

#include "RiuGuiTheme.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiLineEditor.h"

#include <cmath>

CAF_PDM_ABSTRACT_SOURCE_INIT( RimGenericParameter, "GenericParameter" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGenericParameter::RimGenericParameter()
{
    CAF_PDM_InitObject( "Parameter", ":/Bullet.png", "", "" );

    CAF_PDM_InitField( &m_name, "Name", QString(), "Name" );
    m_name.uiCapability()->setUiReadOnly( true );
    m_name.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_label, "Label", QString(), "Name" );
    m_label.uiCapability()->setUiReadOnly( true );
    m_label.uiCapability()->setUiContentTextColor( RiuGuiTheme::getColorByVariableName( "textColor" ) );

    CAF_PDM_InitField( &m_description, "Description", QString(), "Description" );
    m_description.uiCapability()->setUiReadOnly( true );
    m_description.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_advanced, "Advanced", false, "Advanced" );
    m_advanced.uiCapability()->setUiReadOnly( true );
    m_advanced.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_valid, "Valid", false, "Valid" );
    m_valid.uiCapability()->setUiReadOnly( true );
    m_valid.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGenericParameter::~RimGenericParameter()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGenericParameter::setName( QString name )
{
    m_name = name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGenericParameter::setLabel( QString labelText )
{
    m_label = labelText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGenericParameter::setDescription( QString description )
{
    m_description = description;
    m_label.uiCapability()->setUiToolTip( description );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGenericParameter::setAdvanced( bool isAdvanced )
{
    m_advanced = isAdvanced;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGenericParameter::isAdvanced() const
{
    return m_advanced();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimGenericParameter::isValid() const
{
    return m_valid();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGenericParameter::setValid( bool valid )
{
    m_valid = valid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGenericParameter::name() const
{
    return m_name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGenericParameter::label() const
{
    return m_label();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGenericParameter::description() const
{
    return m_description();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGenericParameter::jsonValue() const
{
    return stringValue();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGenericParameter::initAfterRead()
{
    m_label.uiCapability()->setUiToolTip( m_description );
}
