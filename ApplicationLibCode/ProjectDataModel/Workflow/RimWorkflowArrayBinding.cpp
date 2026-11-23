/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimWorkflowArrayBinding.h"

#include "cafPdmUiListEditor.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <cmath>
#include <limits>

CAF_PDM_SOURCE_INIT( RimWorkflowArrayBinding, "WorkflowArrayBinding" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWorkflowArrayBinding::RimWorkflowArrayBinding()
{
    // The JSON schema type of the items; empty for lists edited as JSON text
    CAF_PDM_InitFieldNoDefault( &m_itemType, "ItemType", "Item Type" );
    m_itemType.uiCapability()->setUiHidden( true );

    // The value as JSON text, also for lists edited as lists
    CAF_PDM_InitFieldNoDefault( &m_value, "Value", "Value" );
    CAF_PDM_InitFieldNoDefault( &m_intValues, "IntValues", "Values" );
    CAF_PDM_InitFieldNoDefault( &m_doubleValues, "DoubleValues", "Values" );
    CAF_PDM_InitFieldNoDefault( &m_stringValues, "StringValues", "Values" );

    for ( caf::PdmFieldHandle* field : std::initializer_list<caf::PdmFieldHandle*>{ &m_intValues, &m_doubleValues, &m_stringValues } )
    {
        // The project file format of a list of strings separates the strings by whitespace
        field->xmlCapability()->disableIO();
        field->uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );
        field->uiCapability()->setUiToolTip( "Edit the last row to add a value. Delete removes the selected values, Ctrl-V pastes." );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWorkflowArrayBinding::valueField()
{
    if ( m_itemType() == "integer" ) return &m_intValues;
    if ( m_itemType() == "number" ) return &m_doubleValues;
    if ( m_itemType() == "string" ) return &m_stringValues;
    return &m_value;
}

//--------------------------------------------------------------------------------------------------
/// The default is the current value of the binding, the value in input.yaml or the model default
//--------------------------------------------------------------------------------------------------
void RimWorkflowArrayBinding::applySchema( const QJsonObject& fieldSchema )
{
    const QJsonValue value = fieldSchema.value( "default" );

    m_itemType = fieldSchema.value( "item_type" ).toString();
    if ( value.isArray() && !canEditAsList( value.toArray(), m_itemType() ) ) m_itemType = QString();
    // Text that is not a JSON list, e.g. a list being typed, is kept as JSON text
    if ( value.isString() ) m_itemType = QString();
    RimWorkflowFieldBinding::applySchema( fieldSchema );

    if ( value.isArray() )
    {
        setValues( value.toArray() );
    }
    else if ( value.isString() )
    {
        m_value = value.toString();
    }
}

//--------------------------------------------------------------------------------------------------
/// The list fields are not stored, so they are filled from the JSON text
//--------------------------------------------------------------------------------------------------
void RimWorkflowArrayBinding::initAfterRead()
{
    if ( m_itemType().isEmpty() ) return;

    const QJsonDocument document = QJsonDocument::fromJson( m_value().toUtf8() );
    if ( !document.isArray() ) return;

    if ( !canEditAsList( document.array(), m_itemType() ) ) m_itemType = QString();
    setValues( document.array() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflowArrayBinding::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_intValues || changedField == &m_doubleValues || changedField == &m_stringValues )
    {
        m_value = QString::fromUtf8( QJsonDocument( listValues() ).toJson( QJsonDocument::Compact ) );
    }
    RimWorkflowFieldBinding::fieldChangedByUi( changedField, oldValue, newValue );
}

//--------------------------------------------------------------------------------------------------
/// Whether the list editor holds the values exactly: it has 32-bit integers, and drops empty strings
/// and single spaces as the row for a new value
//--------------------------------------------------------------------------------------------------
bool RimWorkflowArrayBinding::canEditAsList( const QJsonArray& values, const QString& itemType )
{
    for ( const QJsonValue& item : values )
    {
        if ( itemType == "integer" )
        {
            const double value = item.toDouble();
            if ( !item.isDouble() || value != std::trunc( value ) || value < std::numeric_limits<int>::min() ||
                 value > std::numeric_limits<int>::max() )
                return false;
        }
        else if ( itemType == "number" )
        {
            if ( !item.isDouble() ) return false;
        }
        else if ( itemType == "string" )
        {
            if ( !item.isString() || item.toString() == "" || item.toString() == " " ) return false;
        }
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWorkflowArrayBinding::setValues( const QJsonArray& values )
{
    m_value = QString::fromUtf8( QJsonDocument( values ).toJson( QJsonDocument::Compact ) );

    std::vector<int>     ints;
    std::vector<double>  doubles;
    std::vector<QString> strings;
    for ( const QJsonValue& item : values )
    {
        ints.push_back( item.toInt() );
        doubles.push_back( item.toDouble() );
        strings.push_back( item.isString() ? item.toString() : item.toVariant().toString() );
    }
    m_intValues    = ints;
    m_doubleValues = doubles;
    m_stringValues = strings;
}

//--------------------------------------------------------------------------------------------------
/// The values of the list field for the item type
//--------------------------------------------------------------------------------------------------
QJsonArray RimWorkflowArrayBinding::listValues() const
{
    QJsonArray array;
    if ( m_itemType() == "integer" )
    {
        for ( int value : m_intValues() )
            array.append( value );
    }
    else if ( m_itemType() == "number" )
    {
        for ( double value : m_doubleValues() )
            array.append( value );
    }
    else if ( m_itemType() == "string" )
    {
        for ( const QString& value : m_stringValues() )
            array.append( value );
    }
    return array;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QJsonValue RimWorkflowArrayBinding::toJsonValue() const
{
    if ( !hasValue() ) return QJsonValue::Null;
    if ( !m_itemType().isEmpty() ) return listValues();

    const QJsonDocument document = QJsonDocument::fromJson( m_value().toUtf8() );
    return document.isArray() ? QJsonValue( document.array() ) : QJsonValue( m_value() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWorkflowArrayBinding::displayValue() const
{
    const QJsonValue value = toJsonValue();
    if ( !value.isArray() ) return RimWorkflowFieldBinding::displayValue();
    return QString::fromUtf8( QJsonDocument( value.toArray() ).toJson( QJsonDocument::Compact ) );
}
