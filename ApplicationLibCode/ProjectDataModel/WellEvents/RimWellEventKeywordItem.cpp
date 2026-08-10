/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
//  A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RimWellEventKeywordItem.h"

#include "cafPdmUiOrdering.h"

CAF_PDM_SOURCE_INIT( RimWellEventKeywordItem, "RimWellEventKeywordItem" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventKeywordItem::RimWellEventKeywordItem()
{
    CAF_PDM_InitObject( "Keyword Item", "" );

    CAF_PDM_InitFieldNoDefault( &m_itemName, "ItemName", "Item Name" );
    m_itemName.uiCapability()->setUiReadOnly( true );
    CAF_PDM_InitFieldNoDefault( &m_itemType, "ItemType", "Type" );
    m_itemType.uiCapability()->setUiReadOnly( true );
    CAF_PDM_InitFieldNoDefault( &m_stringValue, "StringValue", "String Value" );
    m_stringValue.uiCapability()->setUiHidden( true );
    CAF_PDM_InitFieldNoDefault( &m_intValue, "IntValue", "Integer Value" );
    m_intValue.uiCapability()->setUiHidden( true );
    CAF_PDM_InitFieldNoDefault( &m_doubleValue, "DoubleValue", "Double Value" );
    m_doubleValue.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_valueForUi, "ValueForUi", "Value" );
    m_valueForUi.registerGetMethod( this, &RimWellEventKeywordItem::valueForUi );
    m_valueForUi.registerSetMethod( this, &RimWellEventKeywordItem::setValueForUi );
    m_valueForUi.xmlCapability()->disableIO();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventKeywordItem::~RimWellEventKeywordItem()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellEventKeywordItem::itemName() const
{
    return m_itemName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventKeywordItem::ItemType RimWellEventKeywordItem::itemType() const
{
    return m_itemType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellEventKeywordItem::stringValue() const
{
    return m_stringValue();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellEventKeywordItem::intValue() const
{
    return m_intValue();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellEventKeywordItem::doubleValue() const
{
    return m_doubleValue();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventKeywordItem::setItemName( const QString& name )
{
    m_itemName = name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventKeywordItem::setStringValue( const QString& value )
{
    m_itemType    = ItemType::STRING;
    m_stringValue = value;
    updateValueFieldReadOnlyState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventKeywordItem::setIntValue( int value )
{
    m_itemType = ItemType::INTEGER;
    m_intValue = value;
    updateValueFieldReadOnlyState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventKeywordItem::setDoubleValue( double value )
{
    m_itemType    = ItemType::DOUBLE;
    m_doubleValue = value;
    updateValueFieldReadOnlyState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventKeywordItem::setFlag()
{
    m_itemType = ItemType::FLAG;
    updateValueFieldReadOnlyState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellEventKeywordItem::valueForUi() const
{
    switch ( m_itemType() )
    {
        case ItemType::STRING:
            return m_stringValue();
        case ItemType::INTEGER:
            return QString::number( m_intValue() );
        case ItemType::DOUBLE:
            return QString::number( m_doubleValue() );
        case ItemType::FLAG:
        default:
            return {};
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventKeywordItem::setValueForUi( const QString& value )
{
    switch ( m_itemType() )
    {
        case ItemType::STRING:
            m_stringValue = value;
            break;
        case ItemType::INTEGER:
        {
            bool ok       = false;
            int  intValue = value.toInt( &ok );
            if ( ok ) m_intValue = intValue;
            break;
        }
        case ItemType::DOUBLE:
        {
            bool   ok          = false;
            double doubleValue = value.toDouble( &ok );
            if ( ok ) m_doubleValue = doubleValue;
            break;
        }
        case ItemType::FLAG:
        default:
            break;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventKeywordItem::updateValueFieldReadOnlyState()
{
    m_valueForUi.uiCapability()->setUiReadOnly( m_itemType() == ItemType::FLAG );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventKeywordItem::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_itemName );
    uiOrdering.add( &m_itemType );
    uiOrdering.add( &m_valueForUi );
    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventKeywordItem::initAfterRead()
{
    updateValueFieldReadOnlyState();
}

namespace caf
{
template <>
void AppEnum<RimWellEventKeywordItem::ItemType>::setUp()
{
    addItem( RimWellEventKeywordItem::ItemType::STRING, "STRING", "String" );
    addItem( RimWellEventKeywordItem::ItemType::INTEGER, "INTEGER", "Integer" );
    addItem( RimWellEventKeywordItem::ItemType::DOUBLE, "DOUBLE", "Double" );
    addItem( RimWellEventKeywordItem::ItemType::FLAG, "FLAG", "Flag" );
    setDefault( RimWellEventKeywordItem::ItemType::STRING );
}
} // namespace caf
