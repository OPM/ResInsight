/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RimWorkflowStringBinding.h"

#include "RimWorkflowSchemaTools.h"

#include <QJsonObject>

CAF_PDM_SOURCE_INIT( RimWorkflowStringBinding, "WorkflowStringBinding" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflowStringBinding::RimWorkflowStringBinding()
{
    CAF_PDM_InitFieldNoDefault( &m_value, "Value", "Value" );

    // Untyped (Any) fields read the text as a literal, see RimWorkflowSchemaTools::literalFromText
    CAF_PDM_InitField( &m_isLiteral, "IsLiteral", false, "Is Literal" );
    m_isLiteral.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflowStringBinding::applySchema( const QJsonObject& fieldSchema )
{
    // A binding without a field name is new, and its default is a typed value
    const bool isNew = m_fieldName().isEmpty();
    RimWorkflowFieldBinding::applySchema( fieldSchema );

    const bool wasLiteral = m_isLiteral();
    m_isLiteral           = fieldSchema.value( "type" ).toString() == "any";
    if ( m_isLiteral() )
    {
        QString toolTip = fieldSchema.value( "description" ).toString();
        if ( !toolTip.isEmpty() ) toolTip += "\n\n";
        toolTip += "Any value: true, false, a number, null (None) or JSON. Other text is a string; quote it to keep \"true\" as text.";
        m_value.uiCapability()->setUiToolTip( toolTip );
    }

    if ( fieldSchema.contains( "default" ) )
    {
        const QJsonValue value = fieldSchema.value( "default" );
        if ( !m_isLiteral() )
            m_value = value.toString();
        else if ( value.isNull() )
        {
            // Only an entered None is a value; a model default of None leaves the field unset
            const bool entered = fieldSchema.value( RimWorkflowSchemaTools::explicitNullKey() ).toBool();
            m_value            = entered ? RimWorkflowSchemaTools::textFromLiteral( value ) : QString();
            setHasValue( entered );
        }
        else
        {
            // The text of an existing binding that did not read literals is kept, and now read as a
            // literal. Typed values (input.yaml, model defaults, detached values) keep their type.
            const bool legacyText = !isNew && !wasLiteral && value.isString();
            m_value               = legacyText ? value.toString() : RimWorkflowSchemaTools::textFromLiteral( value );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QJsonValue RimWorkflowStringBinding::toJsonValue() const
{
    if ( !hasValue() ) return QJsonValue( QJsonValue::Null );
    return m_isLiteral() ? RimWorkflowSchemaTools::literalFromText( m_value() ) : QJsonValue( m_value() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWorkflowStringBinding::isExplicitNull() const
{
    return hasValue() && m_isLiteral() && !m_value().trimmed().isEmpty() && toJsonValue().isNull();
}
