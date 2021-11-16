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

#include "RimParameterList.h"

#include "RimListParameter.h"

#include <cmath>

CAF_PDM_SOURCE_INIT( RimParameterList, "ParameterList" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimParameterList::RimParameterList()
{
    CAF_PDM_InitObject( "Parameter List", ":/Bullet.png", "", "" );
    uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_parameterNames, "ParameterNames", "Parameters" );
    m_parameterNames.uiCapability()->setUiHidden( true );
    m_parameterNames.uiCapability()->setUiTreeHidden( true );
    m_parameterNames.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitField( &m_name, "Name", QString(), "Name" );
    m_name.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_label, "Label", QString(), "Name" );
    m_label.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimParameterList::~RimParameterList()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterList::addParameter( QString paramName )
{
    m_parameterNames.v().push_back( paramName );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimParameterList::containsParameter( QString paramName )
{
    for ( auto& param : m_parameterNames.v() )
    {
        if ( param == paramName ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterList::setName( QString name )
{
    m_name = name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimParameterList::setLabel( QString labelText )
{
    m_label = labelText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimParameterList::name() const
{
    return m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimParameterList::label() const
{
    return m_label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimParameterList::parameterValue( QString paramName, std::vector<RimGenericParameter*>& parameters )
{
    for ( auto& param : parameters )
    {
        if ( paramName == param->name() ) return param->stringValue();
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimListParameter* RimParameterList::getAsListParameter( std::vector<RimGenericParameter*> parameters )
{
    QString stringValue = "[";

    bool skipComma = true;

    for ( auto& paramName : m_parameterNames.v() )
    {
        if ( skipComma )
        {
            stringValue += " ";
            skipComma = false;
        }
        else
        {
            stringValue += ", ";
        }
        stringValue += parameterValue( paramName, parameters );
    }

    stringValue += " ]";

    RimListParameter* param = new RimListParameter();
    param->setName( name() );
    param->setLabel( label() );
    param->setValue( stringValue );

    return param;
}
