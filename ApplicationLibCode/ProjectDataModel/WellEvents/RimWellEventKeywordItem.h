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

#pragma once

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QString>

//==================================================================================================
///
/// Represents a single keyword item (field) with a name, type, and value
///
//==================================================================================================
class RimWellEventKeywordItem : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class ItemType
    {
        STRING,
        INTEGER,
        DOUBLE
    };

    RimWellEventKeywordItem();
    ~RimWellEventKeywordItem() override;

    // Getters
    QString  itemName() const;
    ItemType itemType() const;
    QString  stringValue() const;
    int      intValue() const;
    double   doubleValue() const;

    // Setters
    void setItemName( const QString& name );
    void setStringValue( const QString& value );
    void setIntValue( int value );
    void setDoubleValue( double value );

private:
    caf::PdmField<QString>                m_itemName;
    caf::PdmField<caf::AppEnum<ItemType>> m_itemType;
    caf::PdmField<QString>                m_stringValue;
    caf::PdmField<int>                    m_intValue;
    caf::PdmField<double>                 m_doubleValue;
};

namespace caf
{
template <>
void caf::AppEnum<RimWellEventKeywordItem::ItemType>::setUp();
} // namespace caf
