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

#include "FileInterface/RifOpmDeckTools.h"

#include "opm/input/eclipse/Deck/DeckItem.hpp"

CAF_PDM_SOURCE_INIT( RimWellEventKeywordItem, "RimWellEventKeywordItem" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventKeywordItem::RimWellEventKeywordItem()
{
    CAF_PDM_InitObject( "Keyword Item", "" );

    CAF_PDM_InitFieldNoDefault( &m_itemName, "ItemName", "Item Name" );
    CAF_PDM_InitFieldNoDefault( &m_itemType, "ItemType", "Type" );
    CAF_PDM_InitFieldNoDefault( &m_stringValue, "StringValue", "String Value" );
    CAF_PDM_InitFieldNoDefault( &m_intValue, "IntValue", "Integer Value" );
    CAF_PDM_InitFieldNoDefault( &m_doubleValue, "DoubleValue", "Double Value" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventKeywordItem::~RimWellEventKeywordItem() {}

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
    m_itemType = ItemType::STRING;
    m_stringValue = value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventKeywordItem::setIntValue( int value )
{
    m_itemType = ItemType::INTEGER;
    m_intValue = value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventKeywordItem::setDoubleValue( double value )
{
    m_itemType = ItemType::DOUBLE;
    m_doubleValue = value;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Opm::DeckItem RimWellEventKeywordItem::toDeckItem() const
{
    switch ( m_itemType() )
    {
        case ItemType::STRING:
            return RifOpmDeckTools::item( m_itemName().toStdString(), m_stringValue().toStdString() );
        case ItemType::INTEGER:
            return RifOpmDeckTools::item( m_itemName().toStdString(), m_intValue() );
        case ItemType::DOUBLE:
            return RifOpmDeckTools::item( m_itemName().toStdString(), m_doubleValue() );
    }

    return RifOpmDeckTools::item( m_itemName().toStdString(), m_stringValue().toStdString() );
}

namespace caf
{
    template <>
    void AppEnum<RimWellEventKeywordItem::ItemType>::setUp()
    {
        addItem( RimWellEventKeywordItem::ItemType::STRING, "STRING", "String" );
        addItem( RimWellEventKeywordItem::ItemType::INTEGER, "INTEGER", "Integer" );
        addItem( RimWellEventKeywordItem::ItemType::DOUBLE, "DOUBLE", "Double" );
        setDefault( RimWellEventKeywordItem::ItemType::STRING );
    }
} // namespace caf
